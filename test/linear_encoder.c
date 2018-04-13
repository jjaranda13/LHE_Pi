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
    pthread_t camera_down_thread;
    pthread_t down_quantizer_thread;
    pthread_t entropic_streamer_thread;

    options.width = 640;
    options.height = 480;
    options.framerate = 10;
    options.cameraNum = 0;
    options.sensor_mode = 6;

    DEBUG = false;
    yuv_model = 2; // 4:2:0
    pppx = 2;
    pppy = 2;
    uint8_t * hops_obtained, *component_obtained;
    hops_obtained = calloc(1000,sizeof(uint8_t));
    component_obtained = calloc(1000,sizeof(uint8_t));
    int tam_bits;

    camera = init_camera(&options);

    ratio_height_YUV=height_orig_Y/height_orig_UV;
    ratio_width_YUV=width_orig_Y/width_orig_UV;
    pppyUV=2*pppy/ratio_height_YUV;
    pppxUV=2*pppx/ratio_width_YUV;

    init_downsampler();
    init_quantizer();
    init_entropic_enc();
    init_streamer();
    sendH264header();

    for(int j =0; j<100; j++)
    {
        pthread_mutex_lock (&cam_down_mutex);
        pthread_cond_wait (&cam_down_cv,&cam_down_mutex);
        for(int line =0 ; line< height_down_Y; line++)
        {
            downsample_frame_simd(pppx,pppy);
            quantize_frame_normal();
            entropic_enc_frame_normal();

            for (int line=0;line<height_down_Y;line++)
            {
                tam_bits = entropic_enc(hops_Y, bits_Y, line, width_down_Y);
                stream_line(bits_Y, tam_bits, line);
            }
            for (int line=0;line<height_down_UV;line++)
            {
                tam_bits=entropic_enc(hops_U, bits_U, line, width_down_UV);
                stream_line(bits_U, tam_bits, line);
                tam_bits=entropic_enc(hops_V, bits_V, line, width_down_UV);
                stream_line(bits_V, tam_bits, line);
            }
        }
        pthread_mutex_unlock (&cam_down_mutex);
    }
    return 0;
}

