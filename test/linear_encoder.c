/**
 * Test for camera_reader and downsampling integration
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/camera_reader.h"
#include "../include/downsampler.h"
#include "../include/entropic_enc.h"
#include "../include/globals.h"
#include "../include/quantizer.h"
#include "../include/streamer.h"
#include "../include/video_encoder.h"
#include "../player/entropic_decoder.h"
#include "../player/quantizer_decoder.h"

int ratio_height_YUV;
int ratio_width_YUV;
int pppyUV;
int pppxUV;

int main(int argc, char* argv[])
{
    CAMERA_OPTIONS options;
    MMAL_COMPONENT_T *camera;
    int status;

    struct timeval t1, t2;
    double elapsedTime;

    options.width = 640;
    options.height = 480;
    options.framerate = 10;
    options.cameraNum = 0;
    options.sensor_mode = 6;
    yuv_model = 2; // 4:2:0
    pppx = 2;
    pppy = 2;
    DEBUG = true;


    camera = init_camera(&options);
    init_downsampler();
    init_quantizer();
    init_entropic_enc();
    tam_bits_Y = malloc(height_down_Y*sizeof(int));
    tam_bits_U = malloc(height_down_UV*sizeof(int));
    tam_bits_V = malloc(height_down_UV*sizeof(int));
    init_streamer();
    sendH264header();

    for(int j =0; j<500; j++)
    {
        pthread_mutex_lock (&cam_down_mutex);
        pthread_cond_wait (&cam_down_cv,&cam_down_mutex);

        gettimeofday(&t1, NULL);

        downsample_frame_simd(pppx,pppy);

        pthread_mutex_unlock (&cam_down_mutex);

        quantize_frame_normal();
        entropic_enc_frame_normal();
        stream_frame();

        gettimeofday(&t2, NULL);
        elapsedTime += (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    }
    fprintf(stderr,"The time of the loop is %f", elapsedTime/1000);
    return 0;
}



