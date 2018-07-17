void init_image_loader (int width, int height);
void init_image_loader_file (char const* filename);
void load_image(char const* filename);
void save_image(char const* filename, int width, int height, int channels, unsigned char **Y,unsigned char **U, unsigned char **V, int yuvmodel);
