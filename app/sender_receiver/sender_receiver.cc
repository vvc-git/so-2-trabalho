// EPOS Synchronizer Component Test Program

#include <machine.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>
#include <utility/ct_buffer.h>
#include <utility/dt_buffer.h>
#include <utility/string.h>
#include <machine/riscv/riscv_cadence.h>
#include <machine/riscv/riscv_nic.h>

using namespace EPOS;

const int iterations = 1;

// OStream cout;

const int BUF_SIZE = 10000;
const int DMA_SIZE = 4500;
char buffer1[BUF_SIZE];
char buffer2[BUF_SIZE];
Semaphore empty(1);
Semaphore full(0);

char buffer_main[4500];
int qde_frames;

int stack_top_down(char * ptr, int size_datagrama, CT_Buffer* buffer_dma, DT_Buffer* buffer_datagram) {

    // colocando cada frame em um endereço contiguo da memória
    for (int i = 0; i < size_datagrama; i += FRAME_SIZE)
    {
        char frame[FRAME_SIZE];
        memcpy(frame, ptr+i, FRAME_SIZE);
        buffer_dma->alloc_frame(frame);
    }
   
    // desaloca buffer alocado
    buffer_datagram->free(ptr, size_datagrama);

    // Copiando para o buffer da main (Depois de colocar tudo no buffer_main, libera a thread)
    buffer_dma->get_dma_data(buffer_main);

    qde_frames = size_datagrama / FRAME_SIZE;

    return 0;
}


void stack_bottom_up(char * datagrama, CT_Buffer* buffer_dma) {

    // Carrega os frames que chegaram na rede para um buffer da NIC
    buffer_dma->set_dma_data(buffer_main, qde_frames);

    //  Agrupa os frames e coloca em um único datagrama
    buffer_dma->get_dma_data(datagrama);

}


int receiver()
{
    // Cria o buffer nao contiguo para o datagrama
    DT_Buffer* buffer_datagram = new DT_Buffer(buffer2, BUF_SIZE);

    // Cria o buffer contiguo para os frames ethernet
    CT_Buffer* buffer_dma = new CT_Buffer(DMA_SIZE);

    int size_datagrama = 4500;

    for (int i = 0; i < iterations; i++) {

        // Recebe o ponteiro do buffer alocado para os datagramas.
        char * pointer_application = reinterpret_cast<char *> (buffer_datagram->alloc(size_datagrama));

        full.p();

        // instancia um datagrama que vai ser criado para agrupar os frames recebidos
        char datagrama[qde_frames*FRAME_SIZE];

        // datagrama irá receber os dados do buffer_dma
        stack_bottom_up(datagrama, buffer_dma);

        // Copia o datagrama para a região alocada pelo Network_buffer
        memcpy(pointer_application, datagrama, qde_frames*FRAME_SIZE);

        cout << pointer_application[0*FRAME_SIZE] << endl;
        cout << pointer_application[1*FRAME_SIZE] << endl;
        cout << pointer_application[2*FRAME_SIZE] << endl;

        // desaloca buffer alocado
        buffer_datagram->free(pointer_application, size_datagrama);

        empty.v();
    }

    return 0;
}

int sender()
{
    // Cria o buffer nao contiguo para o datagrama
    DT_Buffer* buffer_datagram = new DT_Buffer(buffer1, BUF_SIZE);

    // Cria o buffer contiguo para os frames ethernet
    CT_Buffer* buffer_dma = new CT_Buffer(DMA_SIZE);

    int size_datagrama = 4500;

    for (int i = 0; i < iterations; i++) {

        // Recebe o ponteiro do buffer alocado para os datagramas.
        char * pointer_application = reinterpret_cast<char *> (buffer_datagram->alloc(size_datagrama));
        
        char datagrama[size_datagrama];
        
        datagrama[0*FRAME_SIZE]  = 'A';
        datagrama[1*FRAME_SIZE]  = 'B';
        datagrama[2*FRAME_SIZE]  = 'C';

        // Copia os datagramas para a região alocada pelo DT_Buffer
        memcpy(pointer_application, &datagrama, size_datagrama);
        
        empty.p();
        // Abstrai a pilha de protocolos
        stack_top_down(pointer_application, size_datagrama, buffer_dma, buffer_datagram);
        full.v();

    }
    
    return 0;

}



int main()
{
    cout << "Sender x Receiver" << "\n";

    Cadence_NIC net = Cadence_NIC();
    cout << net.phy_init_tx_desc << "\n";
    cout << net.phy_init_tx_data << "\n";


    // Thread * sen = new Thread(&sender);
    // Thread * rec = new Thread(&receiver);

    // sen->join();
    // rec->join();

    cout << "The end!" << "\n";

    // delete rec;
    // delete sen;

    return 0;
}
