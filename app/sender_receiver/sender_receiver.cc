// EPOS Synchronizer Component Test Program

#include <machine.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>
#include <utility/network_buffer.h>
#include <utility/string.h>

using namespace EPOS;

const int iterations = 128;

OStream cout;

const int BUF_SIZE = 1024;
char buffer[BUF_SIZE];
char buffer2[BUF_SIZE];
Semaphore empty(BUF_SIZE);
Semaphore full(0);

char buffer_main[15000];

int receiver()
{
    Network_buffer* b1 = new Network_buffer(buffer2, 1024);

    full.p();
    b1->set_dma_data(buffer_main);

    char * data = (char *) malloc(1500*3);

    b1->get_dma_data(data);
    
    cout << data[0] << endl;
    
    empty.v();

    return 0;
}

int sender()
{

    Network_buffer* b1 = new Network_buffer(buffer, 1024);
    char * prt = reinterpret_cast<char *> (b1->alloc(512));

    prt[0] ='a';
    prt[2] ='d';

    empty.p();
    memcpy(buffer, prt, 512);
    char frame[1500];
    frame[0]='A';
    b1->alloc_frame(frame);
    frame[0]='B';
    b1->alloc_frame(frame);

    b1->get_dma_data(buffer_main);

    
    
    full.v();

    return 0;

}

int main()
{
    cout << "Sender x Receiver" << "\n";

    Thread * sen = new Thread(&sender);
    sen->join();
    Thread * rec = new Thread(&receiver);

    

   // char * prt =reinterpret_cast<char *> (buffer->alloc(64 * 1024 * 1024));

    // cout << "Endereço buffer: " << buffer->buffer() << "\n";




    // Dado que representa 
    // char data[64 * 1024];
    // data[0] = 'a';
    // data[20] = 'b';
    // int res = buffer->insert(data, 64 * 1024);

    // buffer->remove();

    // cout << "res: " << res << endl;

    
    rec->join();

    cout << "The end!" << "\n";

    // delete buffer;
    // delete rec;
    // delete sen;

    return 0;
}
