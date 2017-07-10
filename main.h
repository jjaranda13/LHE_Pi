

bool DEBUG=true;

char pppx;
char pppy;


//alto y ancho originales del fotograma
int img_height_orig;
int img_width_orig;

//yo creo que una imagen debe ser char **img_Y, char** img_U y char**img_V. poner la palabra scanlines lleva a confusion, ya que una scanline es scanlines[i]
//pero no estoy seguro de cual deberia ser la mejor nomenclatura
//ademas creo que debemos poner unsigned char
char **scanlines;


//arrays de hops resultantes del cuantizador
unsigned char**hops_Y;
unsigned char**hops_U;
unsigned char**hops_V;

unsigned char **result_Y;
unsigned char **result_U;
unsigned char **result_V;
