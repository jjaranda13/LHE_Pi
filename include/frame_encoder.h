double timeval_diff(struct timeval *a, struct timeval *b);
void init_framecoder(int width, int height ,int px, int py) ;
void encode_frame();
void downsample_frame( int pppx,  int pppy);
void downsample_frame_simd( int pppx,  int pppy);
void encode_frame_fromfile();
void quantize_subframe(int start_line,int separacion);
void *quantize_impair();
void *quantize_pair();
void *quantize_one();
void *quantize_two();
void *quantize_three();
void *quantize_four();
void *mytask_target(void *arg);
void quantize_target(unsigned char **res_Y,unsigned char **res_U,unsigned char **res_V);
void encode_file(char filename[]);

int entropic_enc_frame_normal();
void quantize_frame_normal();
