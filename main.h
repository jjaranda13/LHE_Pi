

bool DEBUG;

char pppx;
char pppy;


//alto y ancho originales del fotograma
int img_height_orig;
int img_width_orig;

//joseja: yo creo que una imagen debe ser char **img_Y, char** img_U y char**img_V. poner la palabra scanlines lleva a confusion, ya que una scanline es scanlines[i]
//pero no estoy seguro de cual deberia ser la mejor nomenclatura
//ademas creo que debemos poner unsigned char
char **scanlines;


//arays de imagen delta para video diferencial
unsigned char**delta_Y;
unsigned char**delta_U;
unsigned char**delta_V;

//arrays de hops resultantes del cuantizador
unsigned char**hops_Y;
unsigned char**hops_U;
unsigned char**hops_V;

//arays de imagen resultante tras cuantizar (necesaria para la prediccion de LHE) y para calcular el siguiente delta
unsigned char **result_Y;
unsigned char **result_U;
unsigned char **result_V;
