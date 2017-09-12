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

void *after_task();

int main(int argc, char* argv[])
{
    CAMERA_OPTIONS options;
    MMAL_COMPONENT_T *camera;
    int status;
    pthread_t after_thread;

    options.width = 1280;
    options.height = 720;
    options.framerate = 60;
    options.cameraNum = 0;
    options.sensor_mode = 6;

    DEBUG = false;
    yuv_model = 2; // 4:2:0
    pppx = 2;
    pppy = 1;

    camera = init_camera(&options);

    ratio_height_YUV=height_orig_Y/height_orig_UV;
    ratio_width_YUV=width_orig_Y/width_orig_UV;
    pppyUV=2*pppy/ratio_height_YUV;
    pppxUV=2*pppx/ratio_width_YUV;

    init_downsampler();
    init_quantizer();
    init_entropic_enc();

    status = pthread_create(&after_thread, NULL, after_task, (void *)0);
    if (status)
    {
        printf("Error creating thread %d", status);
    }

    sleep(2);
    status = pthread_cancel(after_thread);
    if (status)
    {
        printf("Error cancelling thread %d", status);
    }
    status = close_camera(camera);
    if (status)
    {
        printf("Error closing camera %d", status);
    }
    return 0;
}


void *after_task(void *argument)
{
    while(1)
    {
        pthread_mutex_lock (&cam_down_mutex);
        pthread_cond_wait (&cam_down_cv,&cam_down_mutex);

        //Downsampling
        for(int y =0 ; y <height_orig_Y; y+=pppy)
        {
            down_avg_horiz(orig_Y,width_orig_Y, orig_down_Y, y,pppx, pppy);
        }
        for(int y =0 ; y <height_orig_UV; y+=pppyUV)
        {
            down_avg_horiz(orig_U,width_orig_UV, orig_down_U, y,pppxUV, pppyUV);
            down_avg_horiz(orig_V,width_orig_UV, orig_down_V, y,pppxUV, pppyUV);
        }

        //Quantizer
        for (int y=0;y<height_down_Y;y++)
        {
            quantize_scanline( orig_down_Y,  y, width_down_Y, hops_Y,result_Y);
        }
        for (int y=0;y<height_down_UV;y++)
        {
            quantize_scanline( orig_down_U, y, width_down_UV, hops_U,result_U);
            quantize_scanline( orig_down_V, y, width_down_UV, hops_V,result_V);
        }

        //Entropic
        for (int y=0;y<height_down_Y;y++)
        {
            entropic_enc(hops_Y, bits_Y, y, width_down_Y);
        }
        for (int y=0;y<height_down_UV;y++){
            entropic_enc(hops_U, bits_U, y, width_down_UV);
            entropic_enc(hops_V, bits_V, y, width_down_UV);
        }
        pthread_mutex_unlock (&cam_down_mutex);
        printf("Line Coded sucessfully\n");
    }
}
