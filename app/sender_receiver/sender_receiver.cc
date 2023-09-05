// EPOS Synchronizer Component Test Program

#include <machine.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>
#include <utility/network_buffer.h>
#include <utility/string.h>

using namespace EPOS;

const int iterations = 128;

// OStream cout;

const int BUF_SIZE = 1024;
char buffer[BUF_SIZE];
Semaphore empty(BUF_SIZE);
Semaphore full(0);

int receiver()
{
    int out = 0;
    //for(int i = 0; i < iterations; i++) {
    full.p();
    cout << "C<-" << buffer[out] << " ";
        // out = (out + 1) % BUF_SIZE;
        // Alarm::delay(100000);
    empty.v();
    //}

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
    full.v();

    return 0;

}

int main()
{
    cout << "Sender x Receiver" << "\n";

    Thread * sen = new Thread(&sender);

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
    sen->join();

    cout << "The end!" << "\n";

    // delete buffer;
    // delete rec;
    // delete sen;

    return 0;
}
