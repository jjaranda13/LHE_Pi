

bool DEBUG;//for execution in debug mode and show more traces

bool downsampler_initialized;
bool quantizer_initialized;


//#define MODULE 8;//default value is 8. used for scanlines processing order

//number of original pixels per sampled pixel
char pppx;
char pppy;


//yuv model
int yuv_model; // 0=444, 1=422, 2=420

//alto y ancho originales del fotograma, antes de ser downsampleado
int height_orig;
int width_orig;
int height_down_Y;
int height_down_UV;
int width_down_Y;
int width_down_UV;

//aray de imagen RGB con 1-4 bytes per pixel
unsigned char *rgb;
int rgb_channels;

//arrays de frame original
unsigned char** orig_Y;
unsigned char** orig_U;
unsigned char** orig_V;

//arrays downsampleados
unsigned char **orig_down_Y;
unsigned char **orig_down_U;
unsigned char **orig_down_V;

//arays de imagen delta para video diferencial
unsigned char **delta_Y;
unsigned char **delta_U;
unsigned char **delta_V;

//arrays de hops resultantes del cuantizador
unsigned char **hops_Y;
unsigned char **hops_U;
unsigned char **hops_V;

//arays de imagen resultante tras cuantizar (necesaria para la prediccion de LHE) y para calcular el siguiente delta
unsigned char **result_Y;
unsigned char **result_U;
unsigned char **result_V;