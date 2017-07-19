void load_frame(char const* filename);

void save_frame(char const* filename, int width, int height, int channels,unsigned char **Y, unsigned char **U,unsigned char **V);

void rgb2yuv(unsigned char *rgb, int rgb_channels);
void yuv2rgb(unsigned char **y, unsigned char **u, unsigned char ** v, int channels, int width, int height, char *data);
double get_PSNR_YUV400();
