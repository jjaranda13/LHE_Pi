void load_frame(char const* filename);

void save_frame(char const* filename, int width, int height, int channels, unsigned char *data);
void rgb2yuv(unsigned char *rgb, unsigned char *y,unsigned char *u, unsigned char *v, int rgb_channels);
