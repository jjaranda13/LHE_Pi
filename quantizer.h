

void init_quantizer();

void quantize_scanline(char **orig_YUV, int line,int width, unsigned char **hops,unsigned char **result_YUV);
void compute_delta(char **orig_YUV, int y,int width, unsigned char **delta) ;
