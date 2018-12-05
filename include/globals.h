#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


#define num_threads 3

#define HTTP_API_PORT 3000

#define DEFAULT_INTELIGENT_DISCARD_MODE 3
#define DEFAULT_FRAME_SKYPPING_MODE 0

#define DEBUG_RANDOM_LOSES 0 // Random looses that are artificially provoqued. Fom 0 qhich means no looses to 100 wich means all loooses.

#define JUMP_TO_EVENS

#define NEON

#define YUV444 0
#define YUV422 1
#define YUV420 2

#define Y_COMPONENT 0
#define U_COMPONENT 1
#define V_COMPONENT 2



int cosa;
bool DEBUG;//for execution in debug mode and show more traces

bool image_loader_initialized;
bool downsampler_initialized;
bool quantizer_initialized;

int inteligent_discard_mode;
int frame_skipping_mode;

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
unsigned char cam_down_flag;

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

//descarte inteligente de scanlines
bool *inteligent_discard_Y;
bool *inteligent_discard_U;
bool *inteligent_discard_V;

// Coding threads: Information to pass to the the coding threads.
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

//coding threds: Declaration
pthread_mutex_t th_done[num_threads];
pthread_t thread[num_threads];

//Streamer: globals variables TODO-> Are those neccesary?
int nal_byte_counter;
int frame_byte_counter;
int total_frames;
int total_bytes;
bool newframe;

// Streamer: threads and mutex for stdout
pthread_t streamer_thread[num_threads*8];
pthread_mutex_t stream_subframe_mutex;

// Streamer:Mutex, flags and conditional variables to sync the streamer.
pthread_cond_t stream_subframe_sync_cv[8];
uint8_t stream_subframe_sync[8];
pthread_mutex_t stream_subframe_sync_mtx;

// Streamer: Information to pass to the the streamer threads
struct thread_streamer_info
{
	int start;
	int separation;
};
struct thread_streamer_info *tsinfo;



// HTTP API: Type for the data and variable to hold it
struct http_socket_info {
	int listener;
	int client_listener;
	struct sockaddr_in server;
	struct sockaddr_in client;
    int client_size;
	char buffer[500];
	int buffer_size;
};
struct http_socket_info http_info;

// Main comand line arguments
bool is_rtp;
int sequence_length;
int sequence_init;

