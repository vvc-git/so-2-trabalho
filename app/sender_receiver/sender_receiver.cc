// EPOS Synchronizer Component Test Program

#include <machine.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>
#include <utility/network_buffer.h>
#include <utility/string.h>

using namespace EPOS;

const int iterations = 1;

OStream cout;

const int BUF_SIZE = 64000;
char buffer1[BUF_SIZE];
char buffer2[BUF_SIZE];
Semaphore empty(1);
Semaphore full(0);

char buffer_main[4500];
int qde_frames;

int stack_top_down( char * ptr, int size_datagrama, Network_buffer* net_buffer) {

    // colocando cada frame em um endereço contiguo da memória
    for (int i = 0; i < size_datagrama; i += FRAME_SIZE)
    {
        char frame[FRAME_SIZE];
        memcpy(frame, ptr+i, FRAME_SIZE);
        net_buffer->alloc_frame(frame);
    }
   
    // desaloca buffer alocado
    net_buffer->free(ptr, size_datagrama);

    // Copiando para o buffer da main (Depois de colocar tudo no buffer_main, libera a thread)
    net_buffer->get_dma_data(buffer_main);

    qde_frames = size_datagrama / FRAME_SIZE;

    return 0;
}


void stack_bottom_up(char * datagrama, Network_buffer* net_buffer) {

    // Carrega os frames que chegaram na rede para um buffer da NIC
    net_buffer->set_dma_data(buffer_main, qde_frames);

    //  Agrupa os frames e coloca em um único datagrama
    net_buffer->get_dma_data(datagrama);

}


int receiver()
{
    // Cria o buffer de gerenciamento de protocolos
    Network_buffer* buffer_managment = new Network_buffer(buffer2, BUF_SIZE);

    int size_datagrama = 4500;

    for (int i = 0; i < iterations; i++) {

        // Recebe o ponteiro do buffer alocado para os datagramas.
        char * pointer_application = reinterpret_cast<char *> (buffer_managment->alloc(size_datagrama));

        full.p();

        // instancia um datagrama que vai ser criado para agrupar os frames recebidos
        char datagrama[qde_frames*FRAME_SIZE];

        stack_bottom_up(datagrama, buffer_managment);

        // Copia os datagramas para a região alocada pelo Network_buffer
        memcpy(pointer_application, datagrama, qde_frames*FRAME_SIZE);

        cout << pointer_application[0*FRAME_SIZE] << endl;
        cout << pointer_application[1*FRAME_SIZE] << endl;
        cout << pointer_application[2*FRAME_SIZE] << endl;

        // desaloca buffer alocado
        buffer_managment->free(pointer_application, size_datagrama);

        empty.v();
    }

    return 0;
}

int sender()
{
    // Cria o buffer de gerenciamento de protocolos
    Network_buffer* buffer_managment = new Network_buffer(buffer1, BUF_SIZE);

    int size_datagrama = 4500;

    for (int i = 0; i < iterations; i++) {


        // Recebe o ponteiro do buffer alocado para os datagramas.
        char * pointer_application = reinterpret_cast<char *> (buffer_managment->alloc(size_datagrama));
        
        char datagrama[size_datagrama];

        
        datagrama[0*FRAME_SIZE]  = 'A';
        datagrama[1*FRAME_SIZE]  = 'B';
        datagrama[2*FRAME_SIZE]  = 'C';

        // Copia os datagramas para a região alocada pelo Network_buffer
        memcpy(pointer_application, &datagrama, size_datagrama);
        
        empty.p();
        // Abstrai a pilha de protocolos
        stack_top_down(pointer_application, size_datagrama, buffer_managment);
        full.v();

    }
    
    return 0;

}



int main()
{
    cout << "Sender x Receiver" << "\n";

    Thread * sen = new Thread(&sender);
    Thread * rec = new Thread(&receiver);

    sen->join();
    rec->join();

    cout << "The end!" << "\n";

    delete rec;
    delete sen;

    return 0;
}
