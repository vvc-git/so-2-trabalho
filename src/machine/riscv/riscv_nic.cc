
#include <machine/riscv/riscv_nic.h>

__BEGIN_SYS

SiFiveU_NIC* SiFiveU_NIC::_device;
SiFiveU_NIC::Interrupt_Id SiFiveU_NIC::_interrupt;

SiFiveU_NIC::SiFiveU_NIC()
{ 
    init_regs();

    Network_buffer::net_buffer->configure_tx_rx();

    address = get_mac();
    
    this->attach(Network_buffer::net_buffer);

    // // TX Alocando memoria para os buffers tx de descritores e dados
    // tx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    // tx_data_buffer = new CT_Buffer(FRAME_SIZE * SLOTS_BUFFER);

    // // RX Alocando memoria para os buffers rx de descritores e dados
    // rx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    // rx_data_buffer = new CT_Buffer(FRAME_SIZE * SLOTS_BUFFER);

    // // Pegando endereço físico dos buffers para NIC
    // // TX
    // tx_desc_phy = tx_desc_buffer->phy_address();
    // tx_data_phy = tx_data_buffer->phy_address();

    // // RX
    // rx_desc_phy = rx_desc_buffer->phy_address();
    // rx_data_phy = rx_data_buffer->phy_address();

    // // Pegando endereço lógico dos buffers para CPU
    // log_init_tx_desc = tx_desc_buffer->log_address();
    // log_init_tx_data = tx_data_buffer->log_address();

    // // Pegando endereço lógico dos buffers para CPU
    // log_init_rx_desc = rx_desc_buffer->log_address();
    // log_init_rx_data = rx_data_buffer->log_address();

    // Phy_Addr addr_desc;
    // Phy_Addr addr_data;


    // db<SiFiveU_NIC>(WRN) << "Endereços utilizados para o rx de dados (Construtor)"<< endl;
    // // setting RX buffers
    // for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    // {
    //     addr_desc = rx_desc_phy + (i * DESC_SIZE);
    //     addr_data = rx_data_phy + (i * FRAME_SIZE);

    //     // Configure Buffer Descriptors, p. 1061
    //     // 3. Mark all entries in this list as owned by controller. Set bit [0] of word [0] of each buffer descriptor to 0.
    //     // 4. Mark the last descriptor in the buffer descriptor list with the wrap bit, (bit [1] in word [0]) set.
    //     // 5. Fill the addresses of the allocated buffers in the buffer descriptors (bits [31-2], Word [0])
    //     Desc *rx_desc = addr_desc;
    //     rx_desc->address = addr_data & RX_WORD0_2_LSB; // Os 2 últimos bits da palavra 0 estao sendo zerados
    //     // db<SiFiveU_NIC>(WRN) << "Endereço:  "<< i << " " << hex << rx_desc->address << endl;

    //     // Setando o bit WRP no último descritor
    //     if (i == (SLOTS_BUFFER - 1)) rx_desc->address = rx_desc->address | RX_WORD0_LSB_WRP;

    //     rx_desc->control = 0;
    // }

    // // setting TX buffers
    // for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    // {
    //     addr_desc = tx_desc_phy + (i * DESC_SIZE);
    //     addr_data = tx_data_phy + (i * FRAME_SIZE);

    //     // Configure BUffer Descriptors, p. 1062
    //     // 2. Mark all entries in this list as owned by the controller. Set bit [31] of word [1] to 0.  (TX_WORD1_OWN_CONTROLLER)
    //     // 3. Mark the last descriptor in the list with the wrap bit. Set bit [30] in word [1] to 1.
    //     // 4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr.
    //     Desc *tx_desc = addr_desc;
    //     tx_desc->address = addr_data;
    //     tx_desc->control = 0;
    //     tx_desc->control = TX_WORD1_OWN_CPU | tx_desc->control;

    //     // Setando o bit WRP no último descritor (item 3)
    //     if (i == (SLOTS_BUFFER - 1)) tx_desc->control = TX_WORD1_WRP_BIT | tx_desc->control;

    // }

    // // Configure Buffer Descriptor, p.1061
    // // 6. Write the base address of this buffer descriptor list to the gem.receive_q{ , 1}_ptr
    // // registers.
    // set_reg(RECEIVE_Q_PTR, rx_desc_phy);

    // // Configure Buffer Descriptor, p.1062
    // //4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr..
    // set_reg(TRANSMIT_Q_PTR, tx_desc_phy);
}

void SiFiveU_NIC::send(Address dst, char* payload, unsigned int payload_size)
{

    if (payload_size <= FRAME_SIZE)
    {
        // Varrer descriptors de tx procurando buffer livre
        // for (; last_desc_idx < SLOTS_BUFFER; )
        // {
        //     Desc *tx_desc = tx_desc_phy + (last_desc_idx * DESC_SIZE);
        //     last_desc_idx = (last_desc_idx + 1) % SLOTS_BUFFER;
            
        //     if (tx_desc->control >> 31) {

        Desc * tx_desc = Network_buffer::net_buffer->get_free_tx_desc();

                
        // Montando o Frame para ser enviado 
        Frame* frame = new (reinterpret_cast<void *>(tx_desc->address)) Frame(this->address, dst, 0x8888, payload, payload_size);
        //memcpy(reinterpret_cast<void *>(tx_desc->address), payload, payload_size);

        // // Seta o tamanho do buffer de dados a ser lido
        // tx_desc->control = tx_desc->control | (payload_size + sizeof(*(frame->header())));

        // // For single buffer ehternet frame, bit[15] of word [1] must also be set.
        // tx_desc->control = (1 << 15) | tx_desc->control;

        // // Coloca o bit 31 como 0 (Bit que indica que a NIC poder ler)
        // tx_desc->control = tx_desc->control & TX_WORD1_OWN_CONTROLLER;


        tx_desc->set_ctrl_transmiting(payload_size + sizeof(*(frame->header())));

        // Habilita a NIC para a execução
        start_transmit();

        // Espera o envio 
        while (!(tx_desc->control & TX_WORD1_OWN_CPU )) ;

        //         // Se conseguiu enviar, sai do loop
        //         break;
        //     }
        // }
        // // Reg32 *add = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + INT_STATUS);
        // // db<SiFiveU_NIC>(WRN) << "INT_STATUS: " << hex << *add << endl;
        return ;
    }
}

void SiFiveU_NIC::receive()
{
    // Desc *desc = rx_desc_phy;

    // unsigned int indx = 0;
    // for (unsigned int i = 0; !(desc->address & RX_OWN); i=(i+1)%SLOTS_BUFFER) {
    //     desc = rx_desc_phy + i * DESC_SIZE;
    //     indx = i;
    //     // db<SiFiveU_NIC>(WRN) << "for -> addr: " << hex << desc->address << endl;
    // }

    // // Pegando payload_size da word[1] do descriptor
    // // unsigned int payload_size = (desc->control & GET_FRAME_LENGTH);
    
    // // Definindo endereço do buffer de dados a partir do índice salvo
    // Reg32 addr = rx_data_phy + indx * FRAME_SIZE; 

    // // Setando o bit WRP no último descritor
    // if (indx == (SLOTS_BUFFER - 1)) desc->address = desc->address | RX_WORD0_LSB_WRP;

    // // Tentando copiar os dados
    // // char data[payload_size];

    // // Network_buffer::net_buffer->alloc_frame(reinterpret_cast<char*>(desc->address));
    
    // // Funcionando
    // // CT_Buffer *buffer = new CT_Buffer(FRAME_SIZE);

    // // Colocando o valor de RX data (addr) para o CT_buffer alocado
    // Network_buffer::net_buffer->buf->save_data_frame(reinterpret_cast<char*>(desc->address));


    // // Setando novamente desc->address, para recebimento de novos frames
    // desc = rx_desc_phy + indx * DESC_SIZE;
    // desc->address = addr;
    
    // // Setando os 2 ultimos bits da word[0]
    // desc->address = desc->address & RX_WORD0_2_LSB; 

    // // Chamando notify (Observed)
    // notify();
}



void SiFiveU_NIC::handle_interrupt() {
    // Setup interrupt for receiving frames
    db<SiFiveU_NIC>(TRC) << "riscv::handle_interrupt "<< endl;

    // Se INT_STATUS[receive_complete] estiver setado, chama receive()
    Reg32 *int_status = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + INT_STATUS);
    if (*int_status & INT_RECEIVE_COMPLETE) {
        db<SiFiveU_NIC>(TRC) << "Interrupt Received" << endl;

        IC::disable(IC::INT_ETH0);

        // Read and clear the gem.int_status[receive_complete] register bit
        // by writing a 1 to the bit in the interrupt handler. Also, read and clear the
        // gem.receive_status register by writing a 1 to gem.receive_status[frame_received] bit.
        set_bits(INT_STATUS, INT_RECEIVE_COMPLETE);
        set_bits(RECEIVE_STATUS, INT_RECEIVE_COMPLETE);

        receive();

        IC::enable(IC::INT_ETH0);
    }
}

void SiFiveU_NIC::int_handler(Interrupt_Id interrupt) {
    db<SiFiveU_NIC>(TRC) << "riscv::int_handler " << interrupt << endl;
     _device->handle_interrupt();
};

void SiFiveU_NIC::init() {
    db<SiFiveU_NIC>(TRC) << "riscv::init "<< endl;

    _device = new (SYSTEM) SiFiveU_NIC();

    _interrupt = IC::INT_ETH0;

    // Install interrupt handler
    IC::int_vector(IC::INT_ETH0, &int_handler);

    // Enable interrupts for device
    IC::enable(IC::INT_ETH0);


}
__END_SYS
