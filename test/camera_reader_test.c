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

    pthread_mutex_lock (&cam_down_mutex);
    pthread_cond_wait (&cam_down_cv,&cam_down_mutex);

    save_frame("../LHE_Pi/img/camera_test_1.bmp", width_orig_Y, height_orig_Y, 1, orig_Y,orig_U,orig_V);

    sleep(1);
    save_frame("../LHE_Pi/img/camera_test_2.bmp", width_orig_Y, height_orig_Y, 1, orig_Y,orig_U,orig_V);

    sleep(1);
    save_frame("../LHE_Pi/img/camera_test_3.bmp", width_orig_Y, height_orig_Y, 1, orig_Y,orig_U,orig_V);

    pthread_mutex_unlock (&cam_down_mutex);

    status = close_camera(camera);
    printf("Status is: %d \nTest completed,check the files generated all should be the same", status);

	return 0;
}
