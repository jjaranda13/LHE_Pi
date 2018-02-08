#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>


#define num_threads 4

int cosa;
bool DEBUG;//for execution in debug mode and show more traces

bool downsampler_initialized;
bool quantizer_initialized;


//#define MODULE 8;//default value is 8. used for scanlines processing order

//number of original pixels per sampled pixel
char pppx;
char pppy;

//GOP size (group of pictures)
char gop_size;
char gop_frame;

//yuv model
int yuv_model; // 0=444, 1=422, 2=420

//alto y ancho originales del fotograma, antes de ser downsampleado
int height_orig_Y;
int width_orig_Y;

int height_orig_UV;
int width_orig_UV;

//ancho y alto de imagen downsampleada
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

//Semaforo para iniciar el downsampler de la imagen capturada
pthread_mutex_t cam_down_mutex;
pthread_cond_t cam_down_cv;

//Variable condicional para iniciar la cuantización de las scanlines(hace broadcast cuando se tenga una slice)
unsigned char down_quant_sem;

//comunicacion down y quantizer
unsigned char *down_quant_flag;

pthread_mutex_t *down_quant_mutex_Y;
pthread_cond_t *down_quant_cv_Y;
pthread_mutex_t *down_quant_mutex_UV;
pthread_cond_t *down_quant_cv_UV;

//Flags de lineas ent y stream
unsigned char *ent_stream_flag;

//Semaforo para comprobar la transmision y enviar si se puede
unsigned char ent_stream_sem;

pthread_mutex_t *ent_stream_mutex_Y;
pthread_cond_t *ent_stream_cv_Y;
pthread_mutex_t *ent_stream_mutex_UV;
pthread_cond_t *ent_stream_cv_UV;


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

uint8_t **bits_Y;
uint8_t **bits_U;
uint8_t **bits_V;

//Tamaño en bytes que ocupa cada linea tras la cuantizacion
int * tam_bits_Y;
int * tam_bits_U;
int * tam_bits_V;


unsigned char *huff_table;
unsigned char rlc_length, condition_length;

unsigned int hops_type[9];




// imagen expandida por EPX ( o por lo que sea)
// su ancho y alto va a ser el de la imagen original. no hace falta explicitarlos
unsigned char **scaled_Y;
unsigned char **scaled_U;
unsigned char **scaled_V;




// bufferes necesarios para video
//------------------------------------
//arrays downsampleados. necesitan ser alocados
unsigned char **last_frame_player_Y;
unsigned char **last_frame_player_U;
unsigned char **last_frame_player_V;

//esto es solo un puntero. no hay que alocarlo. apunta a orig_down o a dif
unsigned char **target_Y;
unsigned char **target_U;
unsigned char **target_V;



//arays de imagen resultante tras cuantizar (necesaria para la prediccion de LHE) y para calcular el siguiente delta
//hay que allocarlos
unsigned char **result2_Y;
unsigned char **result2_U;
unsigned char **result2_V;

//esto es solo un puntero conmutador, no hay que alocarlo. apunta a result o a result2
unsigned char **frame_encoded_Y;
unsigned char **frame_encoded_U;
unsigned char **frame_encoded_V;


//esto lo hacemos global para poder recoger la cantidad de bits del entropico

struct thread_info
{
int start;
int separation;
unsigned char **res_Y;
unsigned char **res_U;
unsigned char **res_V;
int bits_count;
int id;
};
struct thread_info *tinfo;

struct thread_streamer_info
{
int start;
int separation;
};

struct thread_streamer_info *tsinfo;


pthread_mutex_t th_done[num_threads];
pthread_t thread[num_threads];

pthread_t streamer_thread[num_threads*8];

pthread_mutex_t stream_subframe_mutex;
