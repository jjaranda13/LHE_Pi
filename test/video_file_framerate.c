/**
 * Test for camera_reader and downsampling integration
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "../include/globals.h"
#include "../include/downsampler.h"
#include "../include/quantizer.h"
#include "../include/entropic_enc.h"
#include "../include/imgUtil.h"
#include "../include/frame_encoder.h"
#include "../include/video_encoder.h"

// OPTIONS BEGIN/////////
static const char input_filename[] = "/home/pi/test_media/vid/frames_rc/coche_out-%05d.bmp";
static const bool store_result = false;
static const char output_filename[] = "../LHE_Pi/img/out/video/coche-out-%05d.bmp";
static const char interpol[] = "EDGE"; // can be EDGE, EPX or BILINEAR
static const unsigned int number_frames = 1000;
static const unsigned int init_frame = 1;
static const unsigned int discard = 0;
// OPTIONS END/////////

int main(int argc, char* argv[])
{
    char filename[100];
    unsigned long bytes = 0;
    unsigned long total_bytes = 0;
    unsigned int bits = 0;
    long double psnr = 0.0;
    long double total_psnr = 0.0;

    width_orig_Y = 854;
    height_orig_Y = 480;
    width_orig_UV = 427;
    height_orig_UV = 240;

        //---------------------------------------------------------
    orig_Y=malloc(height_orig_Y*sizeof (unsigned char *));
    orig_U=malloc(height_orig_Y*sizeof (unsigned char *));
    orig_V=malloc(height_orig_Y*sizeof (unsigned char *));

    for (int i=0;i<height_orig_Y;i++)
    {
    orig_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
    orig_U[i]=malloc(width_orig_Y* sizeof (unsigned char));
    orig_V[i]=malloc(width_orig_Y* sizeof (unsigned char));
    }

    scaled_Y=malloc(height_orig_Y*sizeof (unsigned char *));
    scaled_U=malloc(height_orig_UV*sizeof (unsigned char *));
    scaled_V=malloc(height_orig_UV*sizeof (unsigned char *));

    for (int i=0;i<height_orig_Y;i++)
    {
        scaled_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
        scaled_U[i]=malloc(width_orig_UV* sizeof (unsigned char));
        scaled_V[i]=malloc(width_orig_UV* sizeof (unsigned char));
    }

    //int encoder
    pppx=2;
    pppy=2;

    init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
    init_videoencoder();
    inteligent_discard_mode = discard;


    for (unsigned int pic = init_frame; pic <init_frame+number_frames; pic++){

        bytes = 0;
        sprintf(filename,input_filename,pic);
        load_frame(filename);
        rgb2yuv(rgb,rgb_channels);
        //Downsampler
        downsample_frame(pppx,pppy);
        free(rgb);
        // Quantizer
        for (int line = 0; line < height_down_Y; line++)
        {
            inteligent_discard_Y[line] = quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
            if(line%2==0)
            {
                inteligent_discard_Y[line] = false;
            }
        }
        for (int line = 0; line < height_down_UV; line++)
        {
            inteligent_discard_U[line] = quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
            inteligent_discard_V[line] = quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
            if(line%2==0)
            {
                inteligent_discard_U[line] = false;
                inteligent_discard_V[line] = false;
            }
        }
        save_frame("../LHE_Pi/img/out/video/test-2.bmp", width_down_Y, height_down_Y, 1, result_Y,result_U,result_V,420);

        // Entropic encoding
        for (int line = 0; line < height_down_Y; line++)
        {
            if (!inteligent_discard_Y[line]) {
                bits = entropic_enc(hops_Y, bits_Y, line, width_down_Y);
                bytes += (bits%8 == 0)? bits/8 : (bits/8)+1;
                bytes += 2; // Header bytes
            }
        }
        for (int line = 0; line < height_down_UV; line++)
        {
            if (!inteligent_discard_U[line]) {
                bits = entropic_enc(hops_U, bits_U, line, width_down_UV);
                bytes += (bits%8 == 0)? bits/8 : (bits/8)+1;
                bytes += 2; // Header bytes
            }
            if (!inteligent_discard_V[line]) {
                bits = entropic_enc(hops_V, bits_V, line, width_down_UV);
                bytes += (bits%8 == 0)? bits/8 : (bits/8)+1;
                bytes += 2; // Header bytes
            }

        }

        // Reconstruct the lost frames
        for (int line = 0; line < height_down_Y; line++)
        {
            if(inteligent_discard_Y[line] == true && line == height_down_Y-1)
            {
                memcpy(result_Y[line], result_Y[line-1], width_down_Y);
            }
            else if(inteligent_discard_Y[line] == true)
            {
                interpolate_scanline(result_Y, line,line-1, line+1, width_down_Y);
            }
        }

        for (int line = 0; line < height_down_UV; line++)
        {
            if(inteligent_discard_U[line] == true && line == height_down_UV-1)
            {
                memcpy(result_U[line], result_U[line-1], width_down_UV);
            }
            else if(inteligent_discard_U[line] == true)
            {
                interpolate_scanline(result_U, line,line-1, line+1, width_down_UV);
            }

            if(inteligent_discard_V[line] == true && line == height_down_UV-1)
            {
                memcpy(result_V[line], result_V[line-1], width_down_UV);
            }
            else if(inteligent_discard_V[line] == true)
            {
                interpolate_scanline(result_V, line,line-1, line+1, width_down_UV);
            }
        }

        if (strcmp(interpol,"EDGE") == 0)
        {
            interpolate_adaptative(result_Y, height_down_Y, width_down_Y, scaled_Y);
            interpolate_adaptative(result_U, height_down_UV, width_down_UV, scaled_U);
            interpolate_adaptative(result_V, height_down_UV, width_down_UV, scaled_V);
        }
        else if (strcmp(interpol,"BILINEAR") == 0)
        {
            interpolate_bilinear(result_Y, height_down_Y, width_down_Y, scaled_Y);
            interpolate_bilinear(result_U, height_down_UV, width_down_UV, scaled_U);
            interpolate_bilinear(result_V, height_down_UV, width_down_UV, scaled_V);
        }
        else if (strcmp(interpol,"EPX") == 0)
        {
            scale_epx(result_Y,height_down_Y,width_down_Y,scaled_Y,38);
            scale_epx(result_U,height_down_UV,width_down_UV,scaled_U,38);
            scale_epx(result_V,height_down_UV,width_down_UV,scaled_V,38);
        }

        if (store_result)
        {
            sprintf(filename,output_filename,pic);
            save_frame(filename, width_orig_Y, height_orig_Y, 1, scaled_Y,scaled_U,scaled_V,420);
        }

        psnr = get_PSNR_Y(scaled_Y,orig_Y, height_orig_Y,width_orig_Y);
        total_psnr += psnr;
        total_bytes += bytes;
        printf("Coded frame %d PSNR=%f, bytes=%d\n", pic, psnr, bytes);

    }
    total_psnr = total_psnr / (float)number_frames;
    total_bytes = total_bytes / number_frames;

    printf("Finished frames=%d PSNR=%f bytes=%d kbps(30fps)=%d\n", number_frames, total_psnr, total_bytes, total_bytes*8*30/1000); // kilo bits per second at 30 frames per second
}
