void load_frame(char const* filename);

void save_frame(char const* filename, int width, int height, int channels,unsigned char **Y, unsigned char **U,unsigned char **V, int yuvmodel);

void rgb2yuv(unsigned char *rgb, int rgb_channels);
void yuv2rgb(unsigned char **y, unsigned char **u, unsigned char ** v, int channels, int width, int height, char *data,int yuvmodel);
double get_PSNR_Y();
