/**
 * Test for camera_reader and downsampling integration
 */
#include <stdio.h>
#include <pthread.h>

#include "../include/camera_reader.h"
#include "../include/downsampler.h"
#include "../include/entropic_enc.h"
#include "../include/globals.h"
#include "../include/quantizer.h"

int ratio_height_YUV;
int ratio_width_YUV;
int pppyUV;
int pppxUV;

void *camera_down_task(void *argument);
void *down_quantizer_task(void *argument);
void *entropic_stream_task(void *argument);

int main(int argc, char* argv[])
{
    ProfilerStart("LHE_Pi_profiling.prof");
    CAMERA_OPTIONS options;
    MMAL_COMPONENT_T *camera;
    int status;
    pthread_t camera_down_thread;
    pthread_t down_quantizer_thread;
    pthread_t entropic_streamer_thread;

    options.width = 1280;
    options.height = 720;
    options.framerate = 60;
    options.cameraNum = 0;
    options.sensor_mode = 6;

    DEBUG = false;
    yuv_model = 2; // 4:2:0
    pppx = 2;
    pppy = 1;

    down_quant_mutex_Y = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t *) *height_down_Y);
    down_quant_cv_Y = (pthread_cond_t *) malloc(sizeof(pthread_cond_t *) *height_down_Y);
    down_quant_mutex_UV = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t *) *height_down_UV);
    down_quant_cv_UV = (pthread_cond_t *) malloc(sizeof(pthread_cond_t *) *height_down_UV);

    ent_stream_mutex_Y = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t *) *height_down_Y);
    ent_stream_cv_Y = (pthread_cond_t *) malloc(sizeof(pthread_cond_t *) *height_down_Y);
    ent_stream_mutex_UV = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t *) *height_down_UV);
    ent_stream_cv_UV = (pthread_cond_t *) malloc(sizeof(pthread_cond_t *) *height_down_UV);

    for (int i=0; i< height_down_Y; i++)
    {
        pthread_mutex_init(&down_quant_mutex_Y[i], NULL);
        pthread_cond_init (&down_quant_cv_Y[i], NULL);
    }
    for (int i=0; i< height_down_UV; i++)
    {
        pthread_mutex_init(&down_quant_mutex_UV[i], NULL);
        pthread_cond_init (&down_quant_cv_UV[i], NULL);
    }

    for (int i=0; i< height_down_Y; i++)
    {
        pthread_mutex_init(&ent_stream_mutex_Y[i], NULL);
        pthread_cond_init (&ent_stream_cv_Y[i], NULL);
    }
    for (int i=0; i< height_down_UV; i++)
    {
        pthread_mutex_init(&ent_stream_mutex_UV[i], NULL);
        pthread_cond_init (&ent_stream_cv_UV[i], NULL);
    }


    camera = init_camera(&options);

    ratio_height_YUV=height_orig_Y/height_orig_UV;
    ratio_width_YUV=width_orig_Y/width_orig_UV;
    pppyUV=2*pppy/ratio_height_YUV;
    pppxUV=2*pppx/ratio_width_YUV;
    init_downsampler();

    init_quantizer();
    init_entropic_enc();

    status = pthread_create(&camera_down_thread, NULL, camera_down_task, (void *)0);
    if (status)
    {
        printf("Error creating thread %d", status);
    }
    status = pthread_create(&down_quantizer_thread, NULL, down_quantizer_task, (void *)0);
    if (status)
    {
        printf("Error creating thread %d", status);
    }
    status = pthread_create(&entropic_streamer_thread, NULL, down_quantizer_task, (void *)0);
    if (status)
    {
        printf("Error creating thread %d", status);
    }

    sleep(2);
    status = pthread_cancel(camera_down_thread);
    if (status)
    {
        printf("Error cancelling thread %d", status);
    }
    status = pthread_cancel(down_quantizer_thread);
    if (status)
    {
        printf("Error cancelling thread %d", status);
    }

    status = close_camera(camera);
    if (status)
    {
        printf("Error closing camera %d", status);
    }
    ProfilerStop();
    return 0;
}


void *camera_down_task(void *argument)
{
    int n = 0;
    int line = 0;
    int separacion = 8;
    int start_line = 0;
    while(1)
    {
        pthread_mutex_lock (&cam_down_mutex);
        pthread_cond_wait (&cam_down_cv,&cam_down_mutex);

        //Downsampling
        for (int i = 0; i<8; i+=pppy)
        {
            n = 0;
            start_line = i;
            line = start_line;
            separacion = 8;
            while (line<height_orig_Y)
            {
                printf("Line %d\n", line/pppy);
                pthread_mutex_lock(&down_quant_mutex_Y[line/pppy]);

                //down_avg_horiz(orig_Y,width_orig_Y, orig_down_Y, line,pppx, pppy);


                //pthread_cond_signal(&down_quant_cv_Y[line/pppy]);
                pthread_mutex_unlock(&down_quant_mutex_Y[line/pppy]);
                n++;
                line=(start_line+n*separacion);
            }

        }
        /*
        for (int i = 0; i<8; i+=pppyUV)
        {
            n = 0;
            start_line = i;
            line =(start_line+n*8)% height_orig_UV;;
            separacion = 8;
            while (line<height_orig_UV)
            {
                pthread_mutex_lock(&down_quant_mutex_UV[line]);

                down_avg_horiz(orig_U,width_orig_UV, orig_down_U, line,pppxUV, pppyUV);
                down_avg_horiz(orig_V,width_orig_UV, orig_down_V, line,pppxUV, pppyUV);

                pthread_cond_signal(&down_quant_cv_UV[line]);
                pthread_mutex_unlock(&down_quant_mutex_UV[line]);
                n++;
                line=(start_line+n*separacion);
            }

        }
        */
        printf("processed frame\n");
        pthread_mutex_unlock (&cam_down_mutex);
    }
}

void *down_quantizer_task(void *argument)
{
    int n = 0;
    int line = 0;
    int start_line = 0;
    int separacion = 8;
    while(1)
    {
        //Quantizer
/*
        for (int i = 0; i<8; i++)
        {
            n = 0;
            start_line = i;
            line = i;
            separacion = 8;
            while (line<height_down_Y)
            {
                pthread_mutex_lock(&down_quant_mutex_Y[line]);
                pthread_cond_wait (&down_quant_cv_Y[line],&down_quant_mutex_Y[line]);

                pthread_mutex_lock(&ent_stream_mutex_Y[line]);

                quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
                entropic_enc(hops_Y, bits_Y, line, width_down_Y);

                pthread_mutex_unlock(&down_quant_mutex_Y[line]);

                pthread_cond_signal(&ent_stream_cv_Y[line]);
                pthread_mutex_unlock(&ent_stream_mutex_Y[line]);
                n++;
                line=(start_line+n*separacion);
            }

        }

        for (int i = 0; i<8; i++)
        {
            n = 0;
            start_line = i;
            line = (start_line+n*8)% height_down_UV;;
            separacion = 8;
            while (line<height_down_UV)
            {
                pthread_mutex_lock(&down_quant_mutex_UV[line]);
                pthread_cond_wait (&down_quant_cv_UV[line],&down_quant_mutex_UV[line]);

                pthread_mutex_lock(&ent_stream_mutex_UV[line]);

                quantize_scanline( orig_down_U, line, width_down_UV, hops_U,result_U);
                quantize_scanline( orig_down_V, line, width_down_UV, hops_V,result_V);
                entropic_enc(hops_U, bits_U, line, width_down_UV);
                entropic_enc(hops_V, bits_V, line, width_down_UV);


                pthread_mutex_unlock(&down_quant_mutex_UV[line]);

                pthread_cond_signal(&ent_stream_cv_UV[line]);
                pthread_mutex_unlock(&ent_stream_mutex_UV[line]);
                n++;
                line=(start_line+n*separacion);
            }
        }
        */
    }
}
void *entropic_stream_task(void *argument)
{
    while(1)
    {

        //void sendData();
    }
}

