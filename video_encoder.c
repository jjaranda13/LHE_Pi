
/**
 * @file video_encoder.c
 * @author JJGA,MAG
 * @date July 2017
 * @brief video coder .
 *
 * description:
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>

#include <pthread.h>
#include "include/globals.h"

#include "include/imgUtil.h"
#include "include/downsampler.h"
#include "include/quantizer.h"
#include "include/file_reader.h"
#include "include/frame_encoder.h"
#include "include/entropic_enc.h"
#include "include/camera_reader.h"
#include "include/video_encoder.h"
#include "include/streamer.h"
#include "include/http_api.h"
#include "include/imgUtil.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))



void init_videoencoder()
{
	tinfo = calloc(num_threads, sizeof(struct thread_info));
	tsinfo = calloc(num_threads*8, sizeof(struct thread_streamer_info));

    frame_skipping_mode = DEFAULT_FRAME_SKYPPING_MODE;

	init_streamer();
}

void encode_video_from_file_sequence(char filename[], int sequence_length)
{
	struct timeval t_ini, t_fin;
	double secs = 0.0, min_secs = 10000.0, max_secs = 0.0, total_secs = 0.0;
    char local_filename[120];
    long bytes = 0;
    int status;
    long sum_discarded_Y=0, sum_discarded_U=0, sum_discarded_V=0 ;

    sprintf(local_filename,filename,sequence_init);
    init_image_loader_file(local_filename);
    init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
    init_videoencoder();
    send_nal();

    for (int pic = sequence_init; pic <sequence_init + sequence_length; pic++)
    {
        sprintf(local_filename,filename,pic);
        status = load_image(local_filename);
        if (status != 0)
        {
            fprintf(stderr,"%s:%s:%d:ERR: Could not open the file %s \n", __FILE__,__func__ ,__LINE__, local_filename);
            return;
        }
        gettimeofday(&t_ini, NULL);
        downsample_frame(pppx,pppy);
        encode_frame(result_Y,result_U,result_V);
        for (int i=0; i< num_threads; i++)
        {
            pthread_join(thread[i], NULL);
        }

        if (DEBUG) gettimeofday(&t_fin, NULL);
        for (int j=0; j< num_threads*8; j++)
        {
            pthread_join(streamer_thread[j], NULL);
        }
        gettimeofday(&t_fin, NULL);

        bytes += frame_byte_counter;
        secs = timeval_diff(&t_fin, &t_ini);
        total_secs += secs;
        min_secs = secs < min_secs? secs :min_secs;
        max_secs = secs > max_secs? secs : max_secs;


        for (int line = 0; line < height_down_Y; line++)
        {
			if (inteligent_discard_Y[line] == true)
			{
				sum_discarded_Y++;
			}
		}
		for (int line = 0; line < height_down_UV; line++)
		{
			if (inteligent_discard_U[line] == true)
			{
				sum_discarded_U++;
			}
			if (inteligent_discard_V[line] == true)
			{
				sum_discarded_V++;
			}
		}
    }
    send_fake_newline();
    send_fake_newline();
    send_fake_newline();
    send_fake_newline();
    send_fake_newline();
    fflush(stdout);

    bytes /= sequence_length;
    total_secs /= sequence_length;
    sum_discarded_Y /= sequence_length;
    sum_discarded_U /= sequence_length;
    sum_discarded_V /= sequence_length;

    fprintf(stderr,"INFO: Sucessfully coded %d images using as average of %d bytes\n The time per frame average=%lfsec max=%lfsec min=%lfsec\n", sequence_length, bytes, total_secs, max_secs, min_secs);
    fprintf(stderr,"INFO: The time per frame average=%lfsec max=%lfsec min=%lfsec\n", total_secs, max_secs, min_secs);
    fprintf(stderr,"INFO: The number of scanlines discarded as average are Y=%d U=%d V=%d\n",sum_discarded_Y, sum_discarded_U, sum_discarded_V);
    return;
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void sendH264header()
{
    //sequence parameter set. afecta a N picture
    const uint8_t sps[] = {0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x0a, 0xf8, 0x41, 0xa2};
    //picture parameter set . afecta a 1 pictures
    const uint8_t pps[] = {0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x38, 0x80};
    const uint8_t frame[] = {0x00, 0x00, 0x00, 0x01, 0x65};
    fwrite("He mandado cabecera H264\n", sizeof(uint8_t), 25, stderr);

    fwrite(&sps,sizeof(uint8_t),11,stdout);
    fwrite(&pps,sizeof(uint8_t),8,stdout);
    fwrite(&frame,sizeof(uint8_t),5,stdout);
    send_frame_header(width_orig_Y, height_orig_Y , pppx, pppy);
    fflush(stdout);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void video_recording()
{
	int status;
    DEBUG=false;
    struct timeval t_ini, t_fin;
    double secs;
    int total_frames=30, total_bits=0, discarded_frames = 0;
    float contador_tiempo=0;
    int i=0;
    float frame_count = 0;

	CAMERA_OPTIONS options;
    MMAL_COMPONENT_T *camera;

    options.width = 640;
    options.height = 480;
    options.framerate = 60;
    options.framerate_div = 2;
    options.cameraNum = 0;
    options.sensor_mode = 7;
    DEBUG = false;
    yuv_model = YUV420;

    camera = init_camera(&options);
    init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
    init_videoencoder();
    init_http_api(HTTP_API_PORT);

    if (is_rtp)
        sendH264header();

    while (true)
    {
        i++;
        if (i==total_frames)
            i=0;

		pthread_mutex_lock (&cam_down_mutex);
		while (cam_down_flag != 1) {
			pthread_cond_wait (&cam_down_cv,&cam_down_mutex);
		}
		cam_down_flag = 0;

		float discard_interval=(float)total_frames/(float)(frame_skipping_mode);
		if (frame_skipping_mode>0)
		{
			frame_count++;
			if (frame_count >discard_interval)
			{
				frame_count-=discard_interval;
				pthread_mutex_unlock (&cam_down_mutex);
				discarded_frames++;
				continue;
			}
		}

        gettimeofday(&t_ini, NULL);
        downsample_frame(pppx,pppy);
		pthread_mutex_unlock (&cam_down_mutex);

        process_http_api();
        encode_frame(result_Y,result_U,result_V);

        for (int i=0; i< num_threads; i++)
        {
            pthread_join(thread[i], NULL);
        }

        for (int j=0; j< num_threads*8; j++)
        {
            pthread_join(streamer_thread[j], NULL);
        }

        gettimeofday(&t_fin, NULL);
        secs = timeval_diff(&t_fin, &t_ini);
        contador_tiempo+=secs;

        if (i == 0)
        {
            fprintf(stderr, " Encoding & Streaming: %4.3g ms Discarded Frames: %d \n", (contador_tiempo/30) * 1000.0, discarded_frames);
            contador_tiempo = 0;
            discarded_frames = 0;

        }
    }
}
