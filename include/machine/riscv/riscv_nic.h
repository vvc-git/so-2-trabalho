#ifndef __pcnet32_h
#define __pcnet32_h

#include <network/ethernet.h>
#include <utility/ct_buffer.h>
#include <architecture/cpu.h>

__BEGIN_SYS

OStream cout;

class Cadence_NIC
{

private:
    friend class Machine_Common;

    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg64 Reg64;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

    // masks
    enum : unsigned int
    {
        RX_WORD0_3_LSB = 0xfffffff8,
        RX_WORD0_3_LSB_WRP = 0x00000002,
    };

    // Utilizando modo de endereçamento de 64 bits
    struct Desc
    {
        volatile Reg32 address_lsb;
        volatile Reg32 control_1;
        volatile Reg32 address_msb; // Upper 32-bit address of the data buffer.
        volatile Reg32 control_3;
    };

public:
    Cadence_NIC();
    ~Cadence_NIC(){};

public:
    CT_Buffer *tx_desc_buffer;
    CT_Buffer *tx_data_buffer;

    CT_Buffer *rx_desc_buffer;
    CT_Buffer *rx_data_buffer;

    Phy_Addr tx_desc_phy;
    Phy_Addr tx_data_phy;

    Phy_Addr rx_desc_phy;
    Phy_Addr rx_data_phy;

    Log_Addr log_init_tx_desc;
    Log_Addr log_init_tx_data;

    unsigned int DATA_SIZE = 1500;
    unsigned int DESC_SIZE = 16;
    unsigned int SLOTS_BUFFER = 5;
};

Cadence_NIC::Cadence_NIC()
{
    // TX Alocando memoria para os buffers tx de descritores e dados
    tx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    tx_data_buffer = new CT_Buffer(DATA_SIZE * SLOTS_BUFFER);

    // RX Alocando memoria para os buffers rx de descritores e dados
    rx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    rx_data_buffer = new CT_Buffer(DATA_SIZE * SLOTS_BUFFER);

    // Pegando endereço físico dos buffers para NIC
    // TX
    tx_desc_phy = tx_desc_buffer->phy_address();
    tx_data_phy = tx_data_buffer->phy_address();

    // RX
    rx_desc_phy = rx_desc_buffer->phy_address();
    rx_data_phy = rx_data_buffer->phy_address();

    // Pegando endereço lógico dos buffers para CPU
    // log_init_tx_desc = buffer_tx_desc->log_address();
    // log_init_tx_data = buffer_tx_data->log_address();

    Phy_Addr addr_desc;
    Phy_Addr addr_data;

    // setting TX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        addr_desc = tx_desc_phy + (i * DESC_SIZE);
        addr_data = tx_data_phy + (i * DATA_SIZE);

        // cout << i << "º \n\n";
        // cout << "addr_desc: " << addr_desc << endl;
        // cout << "addr_data: " << addr_data << endl;

        unsigned int addr_data_lsb = addr_data;         // pegou os 32 menos significativos
        unsigned int addr_data_msb = (addr_data >> 32); // pegou os 32 mais significativos

        // cout << "addr_data_lsb: " << hex << addr_data_lsb << endl;
        // cout << "addr_data_msb: " << hex << addr_data_msb << endl;

        Desc *desc = addr_desc;
        desc->address_lsb = addr_data_lsb;
        desc->control_1 = 7;
        desc->address_msb = addr_data_msb;
        desc->control_3 = 0;

        // cout << "desc: " << desc << endl;
        // cout << "desc->address_lsb: " << hex << desc->address_lsb << endl;
        // cout << "desc->control_1: " << hex << desc->control_1 << endl;
        // cout << "desc->address_msb: " << hex << desc->address_msb << endl;
        // cout << "desc->control_3: " << hex << desc->control_3 << endl;
    }

    // setting RX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        addr_desc = tx_desc_phy + (i * DESC_SIZE);
        addr_data = tx_data_phy + (i * DATA_SIZE);

        // cout << i << "º \n\n";
        // cout << "addr_desc: " << addr_desc << endl;
        // cout << "addr_data: " << addr_data << endl;

        unsigned int addr_data_lsb = addr_data;         // pegou os 32 menos significativos
        unsigned int addr_data_msb = (addr_data >> 32); // pegou os 32 mais significativos

        // cout << "addr_data_lsb: " << hex << addr_data_lsb << endl;
        // cout << "addr_data_msb: " << hex << addr_data_msb << endl;

        Desc *desc = addr_desc;
        desc->address_lsb = addr_data_lsb & RX_WORD0_3_LSB; // Os 3 últimos bits da palavra 0 serão zerados
        desc->control_1 = 7;
        desc->address_msb = addr_data_msb;
        desc->control_3 = 0;

        // Setando o bit WRP no último descritor
        if (i == (SLOTS_BUFFER - 1))
        {
            desc->address_lsb = desc->address_lsb | RX_WORD0_3_LSB_WRP;
        }

        // cout << "desc: " << desc << endl;
        // cout << "desc->address_lsb: " << hex << desc->address_lsb << endl;
        // cout << "desc->control_1: " << hex << desc->control_1 << endl;
        // cout << "desc->address_msb: " << hex << desc->address_msb << endl;
        // cout << "desc->control_3: " << hex << desc->control_3 << endl;
    }
}

__END_SYS

#endif