#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include "include/globals.h"
#include "include/imgUtil.h"
#include "include/file_reader.h"
#include "include/downsampler.h"
#include "include/quantizer.h"
#include "include/frame_encoder.h"
#include "include/entropic_enc.h"
#include "include/streamer.h"
#include "include/video_encoder.h"

#include "stb_image/stb_image_write.h"



double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}

void init_framecoder(int width, int height,int px, int py)
{
    downsampler_initialized=false;
    quantizer_initialized=false;
    pppx = px;
    pppy = py;
    width_orig_Y=width;
    height_orig_Y=height;

    for (int i = 0; i < 9; i++)
        hops_type[i] = 0;

    init_downsampler();
    init_quantizer();
    init_entropic_enc();
}

void encode_slice(int start_line,int separacion,unsigned char **res_Y,unsigned char **res_U,unsigned char **res_V, int *bits_count)
{
    int line=start_line;
    int n=0;

    while (line<height_down_Y)
    {
      inteligent_discard_Y[line]=quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,res_Y);
      tam_bits_Y[line] = entropic_enc(hops_Y, bits_Y, line, width_down_Y);
      *bits_count+=tam_bits_Y[line];

      n++;
      line=(start_line+n*separacion);
    }

    line=start_line;
    n=0;

    line=(start_line+n*8)% height_down_UV;
    while (line<height_down_UV)
    {
      inteligent_discard_U[line]=quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,res_U);
      tam_bits_U[line] = entropic_enc(hops_U, bits_U, line, width_down_UV);
      *bits_count+=tam_bits_U[line];

      inteligent_discard_V[line]=quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,res_V);
      tam_bits_V[line] = entropic_enc(hops_V, bits_V, line, width_down_UV);
      *bits_count+=tam_bits_V[line];

      n++;
      line=(start_line+n*separacion) ;
    }
}

void encode_frame(unsigned char **res_Y,unsigned char **res_U,unsigned char **res_V){

    int status;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < num_threads; i++){
        tinfo[i].start=i*8;
        tinfo[i].separation=8*num_threads;
        tinfo[i].res_Y=res_Y;
        tinfo[i].res_U=res_U;
        tinfo[i].res_V=res_V;
        tinfo[i].bits_count=0;
        tinfo[i].id=i;
    }
    for (int i=0; i< num_threads;i++)
    {
        if ((status=pthread_create(&thread[i], &attr, &encode_frame_threaded, &tinfo[i])))
            fprintf(stderr,"Thread creation failed.");
    }
}

void downsample_frame( int pppx,  int pppy)
{
    int ratio_height_YUV=height_orig_Y/height_orig_UV;
    int ratio_width_YUV=width_orig_Y/width_orig_UV;
    int pppyUV=2*pppy/ratio_height_YUV;
    int pppxUV=2*pppx/ratio_width_YUV;

    if (downsampler_initialized==false)
        init_downsampler();

    for (int line=0;line<height_orig_Y;line+=pppy)
    {
#ifdef NEON
        down_avg_horiz_simd(orig_Y,width_orig_Y,orig_down_Y,line,pppx,pppy);
#else /* NEON */
        down_avg_horiz(orig_Y,width_orig_Y,orig_down_Y,line,pppx,pppy);
#endif /* NEON */
	}
    for (int line=0;line<height_orig_UV;line+=pppyUV)
    {
#ifdef NEON
        down_avg_horiz_simd(orig_U,width_orig_UV,orig_down_U,line,pppxUV,pppyUV);
        down_avg_horiz_simd(orig_V,width_orig_UV,orig_down_V,line,pppxUV,pppyUV);
#else /* NEON */
        down_avg_horiz(orig_U,width_orig_UV,orig_down_U,line,pppxUV,pppyUV);
        down_avg_horiz(orig_V,width_orig_UV,orig_down_V,line,pppxUV,pppyUV);
#endif /* NEON */

    }
}

void quantize_frame_normal()
{
    for (int line=0;line<height_down_Y;line++)
    {
        inteligent_discard_Y[line] = quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
        if(line%2==0)
        {
            inteligent_discard_Y[line] = false;
        }
	}
	for (int line=0;line<height_down_UV;line++)
	{
        inteligent_discard_U[line] = quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
        inteligent_discard_V[line] = quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
        if(line%2==0)
        {
            inteligent_discard_U[line] = false;
            inteligent_discard_V[line] = false;
        }
	}
}

int entropic_enc_frame_normal()
{
    int bits = 0;
    for (int line=0; line<height_down_Y; line++)
    {
        if (inteligent_discard_Y[line] == false)
        {
            tam_bits_Y[line] = entropic_enc(hops_Y, bits_Y, line, width_down_Y);
            bits += tam_bits_Y[line];
        }

    }
    for (int line=0; line<height_down_UV; line++)
    {
        if (inteligent_discard_U[line] == false)
        {
            tam_bits_U[line] = entropic_enc(hops_U, bits_U, line, width_down_UV);
            bits += tam_bits_U[line];
        }
        if (inteligent_discard_V[line] == false)
        {
            tam_bits_V[line] = entropic_enc(hops_V, bits_V, line, width_down_UV);
            bits += tam_bits_U[line];
        }
    }
    if (DEBUG) printf("Bits found in entropic encoding is: %d \n", bits);
    return bits;
}

void *encode_frame_threaded(void *arg)
{
    struct thread_info *tinfo = arg;

    int start=tinfo->start;
    int separation=tinfo->separation;

    int invocaciones=separation/num_threads;

    pthread_mutex_init(&th_done[tinfo->id],NULL);
    pthread_mutex_lock(&th_done[tinfo->id]);


    for (int i=start; i< start+invocaciones;i++)
    {
        encode_slice(i,separation,tinfo->res_Y,tinfo->res_U,tinfo->res_V,&(tinfo->bits_count));
        stream_slice(i, separation);
    }
    pthread_mutex_unlock(&th_done[tinfo->id]);
}

void encode_file(char filename[])
{
    int bits = 0, status, discarded_lines_Y = 0, discarded_lines_U = 0,
		discarded_lines_V = 0;

    init_image_loader_file(filename);
    init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
    init_streamer();
    send_nal();

    status = load_image(filename);
    if (status != 0)
    {
       fprintf(stderr,"%s:%s:%d:ERR: Could not open the file %s \n", __FILE__,__func__ ,__LINE__, filename);
       return;
    }
    downsample_frame(pppx,pppy);
    quantize_frame_normal();
    bits = entropic_enc_frame_normal();
    stream_frame();

    send_fake_newline();
    send_fake_newline();
    send_fake_newline();
    send_fake_newline();
    send_fake_newline();
    fflush(stdout);

    for (int line = 0; line < height_down_Y; line++)
    {
		if (inteligent_discard_Y[line] == true)
		{
			discarded_lines_Y++;
		}
	}
    for (int line = 0; line < height_down_UV; line++)
    {
		if (inteligent_discard_U[line] == true)
		{
			discarded_lines_U++;
		}
		if (inteligent_discard_V[line] == true)
		{
			discarded_lines_V++;
		}
	}
    fprintf(stderr,"INFO: Sucessfully coded %s using %d bytes there were {Y=%d U=%d V=%d} discarded lines\n", filename, (bits%8 == 0)? bits/8 : (bits/8)+1, discarded_lines_Y, discarded_lines_U, discarded_lines_V);
    return;
}


