void init_image_loader (int width, int height);
void init_image_loader_file (char filename[]);

int load_image(char filename[]);
void save_image(char filename[], int width, int height, int channels, unsigned char **Y,unsigned char **U, unsigned char **V, int yuvmodel);
