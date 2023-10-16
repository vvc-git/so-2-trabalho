
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
}

void SiFiveU_NIC::send(Address dst, char* payload, unsigned int payload_size)
{

    if (payload_size <= FRAME_SIZE)
    {

        Desc * tx_desc = Network_buffer::net_buffer->get_free_tx_desc();

        // Montando o Frame para ser enviado 
        Frame* frame = new (reinterpret_cast<void *>(tx_desc->address)) Frame(this->address, dst, 0x8888, payload, payload_size);

        tx_desc->set_ctrl_transmiting(payload_size + sizeof(*(frame->header())));

        // Habilita a NIC para a execução
        start_transmit();

        // Espera o envio 
        while (!(tx_desc->control & TX_WORD1_OWN_CPU )) ;

        return ;
    }
}

void SiFiveU_NIC::receive()
{
    // Notifica o Network buffer para liberar a thread 
    // que retira os dados do rx
    notify();
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
