
#include <machine/riscv/riscv_nic.h>

__BEGIN_SYS

SiFiveU_NIC* SiFiveU_NIC::_device;

SiFiveU_NIC::SiFiveU_NIC()
{ 
    init_regs();

    // TX Alocando memoria para os buffers tx de descritores e dados
    tx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    tx_data_buffer = new CT_Buffer(FRAME_SIZE * SLOTS_BUFFER);

    // RX Alocando memoria para os buffers rx de descritores e dados
    rx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    rx_data_buffer = new CT_Buffer(FRAME_SIZE * SLOTS_BUFFER);

    // Pegando endereço físico dos buffers para NIC
    // TX
    tx_desc_phy = tx_desc_buffer->phy_address();
    tx_data_phy = tx_data_buffer->phy_address();

    // RX
    rx_desc_phy = rx_desc_buffer->phy_address();
    rx_data_phy = rx_data_buffer->phy_address();

    // Pegando endereço lógico dos buffers para CPU
    log_init_tx_desc = tx_desc_buffer->log_address();
    log_init_tx_data = tx_data_buffer->log_address();

    // Pegando endereço lógico dos buffers para CPU
    log_init_rx_desc = rx_desc_buffer->log_address();
    log_init_rx_data = rx_data_buffer->log_address();

    Phy_Addr addr_desc;
    Phy_Addr addr_data;


    db<SiFiveU_NIC>(WRN) << "Endereços utilizados para o rx de dados (Construtor)"<< endl;
    // setting RX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        addr_desc = rx_desc_phy + (i * DESC_SIZE);
        addr_data = rx_data_phy + (i * FRAME_SIZE);

        // Configure Buffer Descriptors, p. 1061
        // 3. Mark all entries in this list as owned by controller. Set bit [0] of word [0] of each buffer descriptor to 0.
        // 4. Mark the last descriptor in the buffer descriptor list with the wrap bit, (bit [1] in word [0]) set.
        // 5. Fill the addresses of the allocated buffers in the buffer descriptors (bits [31-2], Word [0])
        Desc *rx_desc = addr_desc;
        rx_desc->address = addr_data & RX_WORD0_3_LSB; // Os 2 últimos bits da palavra 0 estao sendo zerados
        db<SiFiveU_NIC>(WRN) << "Endereço:  "<< i << " " << hex << rx_desc->address << endl;

        // Setando o bit WRP no último descritor
        if (i == (SLOTS_BUFFER - 1)) rx_desc->address = rx_desc->address | RX_WORD0_3_LSB_WRP;

    }

    // setting TX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        addr_desc = tx_desc_phy + (i * DESC_SIZE);
        addr_data = tx_data_phy + (i * FRAME_SIZE);

        // Configure BUffer Descriptors, p. 1062
        // 2. Mark all entries in this list as owned by the controller. Set bit [31] of word [1] to 0.  (TX_WORD1_OWN_CONTROLLER)
        // 3. Mark the last descriptor in the list with the wrap bit. Set bit [30] in word [1] to 1.
        // 4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr.
        Desc *tx_desc = addr_desc;
        tx_desc->address = addr_data;
        tx_desc->control = 0;
        tx_desc->control = TX_WORD1_OWN_CPU | tx_desc->control;

        // Setando o bit WRP no último descritor (item 3)
        if (i == (SLOTS_BUFFER - 1)) tx_desc->control = TX_WORD1_WRP_BIT | tx_desc->control;

    }

    // Configure Buffer Descriptor, p.1061
    // 6. Write the base address of this buffer descriptor list to the gem.receive_q{ , 1}_ptr
    // registers.
    set_reg(RECEIVE_Q_PTR, rx_desc_phy);

    // Configure Buffer Descriptor, p.1062
    //4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr..
    set_reg(TRANSMIT_Q_PTR, tx_desc_phy);
}

void SiFiveU_NIC::send(Address src, Address dst, char* payload, unsigned int payload_size)
{

    if (payload_size <= FRAME_SIZE)
    {
        // Varrer descriptors de tx procurando buffer livre
        for (; last_desc_idx < SLOTS_BUFFER; )
        {
            Desc *tx_desc = tx_desc_phy + (last_desc_idx * DESC_SIZE);
            last_desc_idx = (last_desc_idx + 1) % SLOTS_BUFFER;
            
            if (tx_desc->control >> 31) {
                
                // Montando o Frame para ser enviado 
                Frame* frame = new (reinterpret_cast<void *>(tx_desc->address)) Frame(this->address, dst, 0x8888, payload, payload_size);
                //memcpy(reinterpret_cast<void *>(tx_desc->address), payload, payload_size);

                // Seta o tamanho do buffer de dados a ser lido
                tx_desc->control = tx_desc->control | (payload_size + sizeof(*(frame->header())));

                // For single buffer ehternet frame, bit[15] of word [1] must also be set.
                tx_desc->control = (1 << 15) | tx_desc->control;

                // Coloca o bit 31 como 0 (Bit que indica que a NIC poder ler)
                tx_desc->control = tx_desc->control & TX_WORD1_OWN_CONTROLLER;

                // Habilita a NIC para a execução
                set_bits(NETWORK_CONTROL, TX_START_PCLK);

                // Espera o envio 
                while (!(tx_desc->control & TX_WORD1_OWN_CPU )) ;

                // Se conseguiu enviar, sai do loop
                break;
            }
        }
        Reg32 *add = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + INT_STATUS);
        db<SiFiveU_NIC>(WRN) << "INT_STATUS: " << hex << *add << endl;
    }
}

void SiFiveU_NIC::receive()
{
    Desc *desc = rx_desc_phy;

    Phy_Addr addr = desc->address;

    CT_Buffer *buffer = new CT_Buffer(FRAME_SIZE);

    // Colocando o valor de RX data (addr) para o CT_buffer alocado
    buffer->set_dma_data((char *)addr, 1);

    // Chamando notify (Observed)
    notify(buffer);
}

void SiFiveU_NIC::receive(Address src, void* payload, unsigned int payload_size)
{
    Desc *desc = rx_desc_phy;
    int indx = 0;

    Reg32 *add = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + INT_STATUS);
    db<SiFiveU_NIC>(WRN) << "INT_STATUS: " << hex << *add << endl;

    for (int i = 0; !(desc->address & RX_OWN); i=(i+1)%SLOTS_BUFFER) {
        desc = rx_desc_phy + i * DESC_SIZE;
        indx = i;
        db<SiFiveU_NIC>(WRN) << "for -> addr: " << hex << desc->address << endl;
    }

    db<SiFiveU_NIC>(WRN) << "Addr final: " << hex << desc->address << endl;
    db<SiFiveU_NIC>(WRN) << "i: " << hex << indx << endl;
    Reg32 addr = rx_data_phy + indx * FRAME_SIZE; 
    db<SiFiveU_NIC>(WRN) << "Addr final: " << hex << addr << endl;

    char data[1500];
  
    memcpy(data, reinterpret_cast<char*>(addr), 1500);

    db<SiFiveU_NIC>(WRN) << "data[0]: " << hex << data[0] << endl;
    db<SiFiveU_NIC>(WRN) << "data[100]: " << hex << data[100] << endl;

    // CT_Buffer *buffer = new CT_Buffer(FRAME_SIZE);

    // Colocando o valor de RX data (addr) para o CT_buffer alocado
    // buffer->set_dma_data(reinterpret_cast<char*>(addr), 100);

    // Chamando notify (Observed)
    // notify(buffer);
}

void SiFiveU_NIC::init_regs() 
{
    // 1. Clear the network control register
    //  Write 0x0 to the gem.network_control register.
    set_reg(NETWORK_CONTROL, 0x0);

    // 2. Clear the statistics registers
    // Write a 1 to the gem.network_control [clear_all_stats_regs].
    set_bits(NETWORK_CONTROL, CLEAR_ALL_STATS_REGS);

    // 3. Clear the status registers. Write a 1 to the status registers.
    // gem.receive_status = 0x0F
    set_reg(RECEIVE_STATUS, 0x0F);

    // gem.transmit_status = 0xFF.
    set_reg(TRANSMIT_STATUS, 0xFF);

    // 4.Disable all interrupts.
    // Write 0x7FF_FEFF to the gem. int_disable register.
    set_reg(INT_DISABLE, 0x7FFFEFF);

    // 5. Clear the buffer queues.
    // Write 0x0 to the gem.transmit_q{}_ptr
    set_reg(TRANSMIT_Q_PTR, 0x0);

    // Write 0x0 to the gem.transmit_q{1}_ptr
    set_reg(TRANSMIT_Q1_PTR, 0x0);

    // Write 0x0 to the gem.receive_q{}_ptr
    set_reg(RECEIVE_Q_PTR, 0x0);

     // Write 0x0 to the gem.receive_q{1}_ptr
    set_reg(RECEIVE_Q1_PTR, 0x0);

    // Configure the controller

    // 1. Enable full duplex.
    // a. Write a 1 to the gem.network_config[full_duplex] bit.
    set_bits(NETWORK_CONFIG, FULL_DUPLEX);

    // b. Enable gigabit mode.
    // Write a 1 to the gem.network_config[gigabit_mode_enable] bit.
    set_bits(NETWORK_CONFIG, GIGABIT_MODE_ENABLE);

    // c. Enable reception of broadcast or multicast frames.
    // Write a 0 to the gem.network_config[no_broadcast] register to enable broadcast frames.
    set_bits_and(NETWORK_CONFIG, NO_BROADCAST);


    // write a 1 to the gem.network_config[multicast_hash_en] bit to enable multicast frames.
    set_bits(NETWORK_CONFIG, MULTICAST_HASH_ENABLE);

    // d. Enable promiscuous mode.
    // Write a 1 to the gem.network_config[copy_all_frames] bit.
    set_bits(NETWORK_CONFIG, COPY_ALL_FRAMES);

    // e. Enable TCP/IP checksum offload feature on receive.
    // Write a 1 to the gem.network_config[receive_checksum_offload_enable] bit.
    set_bits(NETWORK_CONFIG, RECEIVE_CHECKSUM_OFFLOAD_ENABLE);

    // f.Enable pause frames. Write a 1 to gem.network_config[pause_enable] bit.
    set_bits(NETWORK_CONFIG, PAUSE_ENABLE);

    // g. Set the MDC clock divisor.
    // Write the appropriate MDC clock divisor to the gem.network_config[mdc_clock_division] bit.
    set_bits(NETWORK_CONFIG, MDC_CLOCK_DIVISION);

    // 2. Set the MAC address.
    // Write to the gem.spec_add1_bottom register.
    Reg32 * low = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + SPEC_ADD1_BOTTOM);
    set_reg(SPEC_ADD1_BOTTOM, *low);
    
    
    // Write to the gem.spec_add1_top register. The
    // most significant 16 bits go to gem.spec_add1_top
    Reg32 * high = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + SPEC_ADD1_TOP);
    set_reg(SPEC_ADD1_TOP, *high);

    mac_low = * low;
    mac_high = * high;

    address[0] = *low >> 0;
    address[1] = *low >> 8;
    address[2] = *low >> 16;
    address[3] = *low >> 24;
    address[4] = *high >> 0;
    address[5] = *high >> 8;


    // 3. Program the DMA configuration register (gem.dma_config)

    // a. Set the receive buffer size to 1,600 bytes. Write a value of 8'h19 to the
    // gem.dma_config[rx_buf_size] bit field. (escrevendo 24, pois 24*64 = 1536)
    set_reg(DMA_CONFIG, RX_BUF_SIZE);

    // b. Set the receiver packet buffer memory size to the full configured addressable space
    // of 32 KB. Write 2'b11 to the gem.dma_config[rx_pbuf_size] bit field
    set_bits(DMA_CONFIG, RX_PBUF_SIZE);

    // C. Set the transmitter packet buffer memory size to the full configured addressable
    // space of 32 KB. Write a 1 to the gem.dma_config[tx_pbuf_size] bit.
    set_bits(DMA_CONFIG, TX_PBUF_SIZE);

    // d. Enable TCP/IP checksum generation offload on the transmitter. Write a 1 to the
    // gem.dma_config[tx_pbuf_tcp_en] bit.
    set_bits(DMA_CONFIG, TX_PBUF_TCP_EN);

    // e. Configure for a little endian system. Write a 0 to the
    // gem.dma_config[endian_swap_packet] bit.
    set_bits_and(DMA_CONFIG, ENDIAN_SWAP_PACKET);

    // f. Configure AXI fixed burst length. Write 5'h10 to the
    // gem.dma_config[amba_burst_length] bit field to use INCR16 AXI burst for higher
    // performance.
    set_bits(DMA_CONFIG, AMBA_BURST_LENGTH);

    // 4. Program the network control register (gem.network_control).
    // a. Enable the MDIO. Write a 1 to the gem.network_control[man_port_en] bit.
    set_bits(NETWORK_CONTROL, MAN_PORT_EN);
    
    // b. Enable the transmitter. Write a 1 to the gem.network_control[enable_transmit] bit.
    set_bits(NETWORK_CONTROL, ENABLE_TRANSMIT);
    
    // c. Enable the receiver. Write a 1 to the gem.network_control[enable_receive] bit.
    set_bits(NETWORK_CONTROL, ENABLE_RECEIVE);

    set_reg(INT_ENABLE, 0x2fffffff); // habilitando todas as interrupcoes
    //set_reg(INT_ENABLE, INT_TRASNMIT_COMPLETE | INT_RECEIVE_OVERRUN | INT_RECEIVE_COMPLETE);

}

void SiFiveU_NIC::handle_interrupt() {
    // Setup interrupt 
    return;

}
__END_SYS
