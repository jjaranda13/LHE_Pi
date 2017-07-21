

void init_quantizer();
void close_quantizer();
void quantize_scanline(unsigned char **orig_YUV, int line,int width, unsigned char **hops,unsigned char **result_YUV);
