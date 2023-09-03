// EPOS Synchronizer Component Test Program

#include <machine.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>
#include <utility/network_buffer.h>

using namespace EPOS;

const int iterations = 128;

// OStream cout;

const int BUF_SIZE = 16;
char buffer[BUF_SIZE];
// Semaphore empty(BUF_SIZE);
// Semaphore full(0);

int receiver()
{
    // int out = 0;
    for(int i = 0; i < iterations; i++) {
        // full.p();
        // cout << "C<-" << buffer[out] << " ";
        // out = (out + 1) % BUF_SIZE;
        // Alarm::delay(100000);
        // empty.v();
    }

    return 0;
}

int sender()
{
    // producer
    // int in = 0;
    for(int i = 0; i < iterations; i++) {
        // empty.p();
        // Alarm::delay(100000);

        // Alocar buffer em nivel de aplicação
        

        // Alocar buffer em nivel ethernet

        // buffer[in] = 'a' + in;
        // cout << "P-> " << buffer[in] << " ";
        // in = (in + 1) % BUF_SIZE;
        // full.v();
    }
    return 0;

}

int main()
{
    cout << "Sender x Receiver" << "\n";

    Thread * rec = new Thread(&receiver);
    Thread * sen = new Thread(&sender);

    Network_buffer* buffer = new Network_buffer(10 * 16 * 64 * 1024);
    // cout << "Endereço buffer: " << buffer->buffer() << "\n";

    char data = 'a';
    int res = buffer->insert(&data, 64 * 1024);

    cout << "res: " << res << endl;

    rec->join();
    sen->join();

    cout << "The end!" << "\n";

    delete buffer;
    delete rec;
    delete sen;

    return 0;
}
