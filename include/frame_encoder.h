
void init_framecoder(int width, int height ,int px, int py) ;
void encode_frame();
void downsample_frame( int pppx,  int pppy);
void encode_frame_fromfile();
void *quantize_impair();
void *quantize_pair();
void *quantize_one();
void *quantize_two();
void *quantize_three();
void *quantize_four();
