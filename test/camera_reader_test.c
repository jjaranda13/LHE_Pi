/**
 * Test for camera_reader
 */

#include "../include/camera_reader.h"
#include "../include/globals.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[])
{
    CAMERA_OPTIONS options;
    MMAL_COMPONENT_T *camera;
    int status, y;
    options.width = 1280;
    options.height = 720;
    options.framerate = 60;
    options.cameraNum = 0;
    options.sensor_mode = 6;
    DEBUG = false;
    yuv_model = 2; // 4:2:0

    camera = init_camera(&options);

    int t1;
    double time_to_run = 5.0;
    time_t start, end;
    char str[80]= "/home/pi/Desktop/";
    char t1s[80];
    FILE *fp;

    time (&start);
    time (&end);
    while(difftime(end, start) < time_to_run) // Run for 10 seconds
    {
        pthread_mutex_lock (&cam_down_mutex);
        pthread_cond_wait (&cam_down_cv,&cam_down_mutex);

        t1 = clock();
        snprintf(t1s,80, "%d", t1);
        fp=fopen(strcat(str,t1s), "w+");
        for(y =0 ; y <height_orig_Y; y++)
        {
            fwrite(orig_Y[y], 1, width_orig_Y, fp);
        }
        for(y =0 ; y <height_orig_UV; y++)
        {
            fwrite(orig_U[y], 1, width_orig_UV, fp);
        }
        for(y =0 ; y <height_orig_UV; y++)
        {
            fwrite(orig_V[y], 1, width_orig_UV, fp);
        }
        pthread_mutex_unlock (&cam_down_mutex);

        fclose(fp);
        time (&end);
    }

    status = close_camera(camera);
    printf("Status is: %d \nTest completed,check the files generated ", status);
	return 0;
}
