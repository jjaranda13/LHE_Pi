

void init_quantizer();
void close_quantizer();
bool quantize_scanline(unsigned char **orig_YUV, int y,int width, unsigned char **hops,unsigned char **result_YUV);
