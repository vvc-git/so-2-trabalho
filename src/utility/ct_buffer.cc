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

void CT_Buffer::get_data_frame(char * data)
{
    if (next <= 0) {return;};
    --next;
    char * addr = base + next * FRAME_SIZE;
    memcpy(data, addr, FRAME_SIZE);
   
    return;
    

    
};

// Salva o dado do frame
void CT_Buffer::save_data_frame(char *data)
{
    db<SiFiveU_NIC>(WRN) << "antes do if do save" <<  endl;

    if (next < size) {
        char * addr = base + next * FRAME_SIZE;
        memcpy(addr, data, FRAME_SIZE);
        ++next; 
    }
};


__END_UTIL
