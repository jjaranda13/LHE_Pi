/**
 * Test for camera_reader and downsampling integration
 */
#include <stdio.h>
#include <pthread.h>

#include "../include/camera_reader.h"
#include "../include/downsampler.h"
#include "../include/globals.h"

void *downsampling_task();

int ratio_height_YUV;
int ratio_width_YUV;
int pppyUV;
int pppxUV;

int main(int argc, char* argv[])
{
    //ProfilerStart("LHE_Pi_profiling.prof");
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
    pppy = 2;

    camera = init_camera(&options);
    ratio_height_YUV=height_orig_Y/height_orig_UV;
    ratio_width_YUV=width_orig_Y/width_orig_UV;
    pppyUV=2*pppy/ratio_height_YUV;
    pppxUV=2*pppx/ratio_width_YUV;
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
    //ProfilerStop();
    return 0;
}


void *downsampling_task(void *argument)
{
    struct timeval t_ini, t_fin;
    double secs;
    int veces =100;
    while(1)
    {
        pthread_mutex_lock (&cam_down_mutex);
        pthread_cond_wait (&cam_down_cv,&cam_down_mutex);
        gettimeofday(&t_ini, NULL);
        for (int i =0; i< veces; i++){


            for(int y =0 ; y <height_orig_Y; y+=pppy)
            {
                down_avg_horiz(orig_Y,width_orig_Y, orig_down_Y, y,pppx, pppy);
            }

            for(int y =0 ; y <height_orig_UV; y+=pppyUV)
            {
                down_avg_horiz(orig_U,width_orig_UV, orig_down_U, y,pppxUV, pppyUV);
                down_avg_horiz(orig_V,width_orig_UV, orig_down_V, y,pppxUV, pppyUV);
            }
        }
        gettimeofday(&t_fin, NULL);
        printf("Time in microseconds: %ld microseconds\n",(((t_fin.tv_sec - t_ini.tv_sec)*1000000L+t_fin.tv_usec) - t_ini.tv_usec)/veces);
        //save_frame("../LHE_Pi/img/orig_down_Y.bmp", width_down_Y, height_down_Y, 1, orig_down_Y,orig_down_U,orig_down_V);
        exit(0);
        pthread_mutex_unlock (&cam_down_mutex);
    }

}
