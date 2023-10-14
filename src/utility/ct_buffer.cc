#include <utility/ct_buffer.h>


__BEGIN_UTIL

// alocando os frames no dma_buffer
void CT_Buffer::alloc_frame(char frame[])
{
    if (count_frames <= buffer_size / FRAME_SIZE)
    {
        memcpy(dma_ptr, frame, FRAME_SIZE);
        dma_ptr += FRAME_SIZE;
        ++count_frames;
    }
};

void CT_Buffer::get_dma_data(char *dma_data)
{
    // serão copiados apenas os frames alocados, por enquanto o dma_buffer inteiro
    memcpy(dma_data, dma.log_address(), count_frames * FRAME_SIZE);
    dma_ptr = dma.log_address();
    count_frames = 0;
};

// Recebe os dados da rede.
// Por enquanto, preenche todo o dma buffer, seta o ponteiro dma para o final do buffer
// e seta count_frames para o valor máximo
void CT_Buffer::set_dma_data(char *dma_data, int amnt_frames)
{
    unsigned int bytes = amnt_frames * FRAME_SIZE;
    memcpy(dma.log_address(), dma_data, bytes);
    dma_ptr = dma.log_address() + bytes;
    count_frames = amnt_frames;
};


unsigned int CT_Buffer::allocated()
{
    return count_frames;
}

__END_UTIL
