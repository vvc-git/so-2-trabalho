#include <utility/ct_buffer.h>


__BEGIN_UTIL

// // alocando os frames no dma_buffer
// void CT_Buffer::alloc(char frame[])
// {
//     if (count_frames <= buffer_size / FRAME_SIZE)
//     {
//         memcpy(first, frame, FRAME_SIZE);
//         first += FRAME_SIZE;
//         ++count_frames;
//     }
// };

void CT_Buffer::get_data_frame(char * data, unsigned int size)
{
    if (next <= 0) {return;};
    --next;
    char * addr = base + next * size;
    memcpy(data, addr, size);
   
    return;
    

    
};

// Salva o dado do frame
void CT_Buffer::save_data_frame(char *data, unsigned int size)
{

    if (next < size) {
        char * addr = base + next * size;
        memcpy(addr, data, size);
        ++next; 
    }
};


__END_UTIL
