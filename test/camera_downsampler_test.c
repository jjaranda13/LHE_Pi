/**
 * Test for camera_reader and downsampling integration
 */
#include <stdio.h>
#include <pthread.h>

#include "../include/camera_reader.h"
#include "../include/downsampler.h"
#include "../include/globals.h"

void *downsampling_task();

int main(int argc, char* argv[])
{
    CAMERA_OPTIONS options;
    MMAL_COMPONENT_T *camera;
    int status;
    pthread_t downsampling_thread;

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
    init_downsampler();

    status = pthread_create(&downsampling_thread, NULL, downsampling_task, (void *)0);
    if (status)
    {
        printf("Error creating thread %d", status);
    }

    sleep(2);
    status = pthread_cancel(downsampling_thread);
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


void *downsampling_task(void *argument)
{
    while(1)
    {
        if (cam_down_sem==1)
        {
            for(int y =0 ; y <height_orig_Y; y++)
            {
                down_avg_horiz(orig_Y,width_orig_Y, orig_down_Y, y,pppx, pppy);
            }
            for(int y =0 ; y <height_orig_UV; y++)
            {
                //down_avg_horiz(orig_U,width_orig_UV, orig_down_U, y,pppx, pppy); //Those has been commented in order to obtain some processed frames
            }
            for(int y =0 ; y <height_orig_UV; y++)
            {
                //down_avg_horiz(orig_V,width_orig_UV, orig_down_V, y,pppx, pppy); //Those has been commented in order to obtain some processed frames
            }
            cam_down_sem=0;
            //printf("Line downsampled");
        }
    }

}
