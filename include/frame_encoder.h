double timeval_diff(struct timeval *a, struct timeval *b);
void init_framecoder(int width, int height ,int px, int py) ;
void encode_slice(int start_line,int separacion,unsigned char **res_Y,unsigned char **res_U,unsigned char **res_V, int *bits_count);
void encode_frame(unsigned char **res_Y,unsigned char **res_U,unsigned char **res_V);
void downsample_frame( int pppx,  int pppy);
void quantize_frame_normal();
int entropic_enc_frame_normal();
void *encode_frame_threaded(void *arg);
void encode_file(char filename[]);
