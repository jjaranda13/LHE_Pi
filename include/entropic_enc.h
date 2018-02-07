typedef struct PutBitContext {
    uint32_t bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
    int size_in_bits;
} PutBitContext;


void init_huffman_table();
void init_entropic_enc();
static inline void init_put_bits(PutBitContext *s, uint8_t *buffer, int buffer_size);
static inline void put_bits(PutBitContext *s, int n, unsigned int value);
static inline int put_bits_count(PutBitContext *s);

/**
 * @brief Fushes the integer used to the bitstream.
 *
 * This function flushes the integer which is being used to memory. It adds 32
 * bits to the end of the stream and respects the bit_left in order to be able
 * to calculate the extact bits used. Some padding up to 32 bits will be added
 * at the end of the stream
 *
 * @param s PutBitContext that has to be flushed.
 */
static inline void put_bits_flush(PutBitContext *s);
int entropic_enc(unsigned char **hops, uint8_t **bits, unsigned int line, unsigned int line_width);
