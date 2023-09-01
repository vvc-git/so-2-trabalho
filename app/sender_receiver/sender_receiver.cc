// EPOS Synchronizer Component Test Program

#include <machine.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>

using namespace EPOS;

const int iterations = 128;

OStream cout;

const int BUF_SIZE = 16;
char buffer[BUF_SIZE];
// Semaphore empty(BUF_SIZE);
// Semaphore full(0);

int receiver()
{
    int out = 0;
    for(int i = 0; i < iterations; i++) {
        // full.p();
        cout << "C<-" << buffer[out] << " ";
        out = (out + 1) % BUF_SIZE;
        Alarm::delay(100000);
        // empty.v();
    }

    return 0;
}

int sender()
{
    // producer
    int in = 0;
    for(int i = 0; i < iterations; i++) {
        // empty.p();
        Alarm::delay(100000);

        // Alocar buffer em nivel de aplicação
        

        // Alocar buffer em nivel ethernet

        buffer[in] = 'a' + in;
        cout << "P-> " << buffer[in] << " ";
        in = (in + 1) % BUF_SIZE;
        // full.v();
    }
    return 0;

}

int main()
{
    cout << "Sender x Receiver" << endl;

    Thread * rec = new Thread(&receiver);
    Thread * sen = new Thread(&sender);

    MyBuffer buffer = new Mybuffer(size);
    buffer.insert(objeto);

    rec->join();
    sen->join();

    cout << "The end!" << endl;

    delete rec;
    delete sen;

    return 0;
}
