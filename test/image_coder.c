/**
 * Test for camera_reader and downsampling integration
 */
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "../include/globals.h"
#include "../include/downsampler.h"
#include "../include/quantizer.h"
#include "../include/imgUtil.h"

static const char input_filename[] = "../LHE_Pi/img/lena.bmp";//"/home/pi/test_media/vid/frames/bunny-00300.bmp";//"../LHE_Pi/img/lena.bmp";
static const char output_filename[] = "../LHE_Pi/img/out/lena_dec.bmp";//"../LHE_Pi/img/out/video/bunny-out-00300.bmp";//"../LHE_Pi/img/out/lena_dec.bmp";

int main(int argc, char* argv[])
{

    unsigned int discarded_lines = 0;

    pppx = 2;
    pppy = 2;
    yuv_model = 2;

    DEBUG = false;

    load_frame(input_filename);

    orig_Y=malloc(height_orig_Y*sizeof (unsigned char *));
    orig_U=malloc(height_orig_Y*sizeof (unsigned char *));
    orig_V=malloc(height_orig_Y*sizeof (unsigned char *));

    for (int i=0;i<height_orig_Y;i++)
    {
    orig_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
    orig_U[i]=malloc(width_orig_Y* sizeof (unsigned char));
    orig_V[i]=malloc(width_orig_Y* sizeof (unsigned char));
    }


    rgb2yuv(rgb,rgb_channels);
    init_downsampler();
    init_quantizer();
    inteligent_discard_mode = 0;



    scaled_Y=malloc(height_orig_Y*sizeof (unsigned char *));
    scaled_U=malloc(height_orig_Y*sizeof (unsigned char *));
    scaled_V=malloc(height_orig_Y*sizeof (unsigned char *));

    for (int i=0;i<height_orig_Y;i++)
    {
        scaled_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
        scaled_U[i]=malloc(height_orig_Y* sizeof (unsigned char));
        scaled_V[i]=malloc(height_orig_Y* sizeof (unsigned char));
    }

    downsample_frame_simd(pppx,pppy);

    for (int line = 0; line < height_down_Y; line++)
    {
        inteligent_discard_Y[line] = quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
        if(line%2==0)
        {
            inteligent_discard_Y[line] = false;
        }
        if(inteligent_discard_Y[line] == true)
            discarded_lines++;

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

    /*interpolate_adaptative(result_Y, height_down_Y, width_down_Y, scaled_Y);
    interpolate_adaptative(result_U, height_down_UV, width_down_UV, scaled_U);
    interpolate_adaptative(result_V, height_down_UV, width_down_UV, scaled_V);*/

    /*interpolate_bilinear(result_Y, height_down_Y, width_down_Y, scaled_Y);
    interpolate_bilinear(result_U, height_down_UV, width_down_UV, scaled_U);
    interpolate_bilinear(result_V, height_down_UV, width_down_UV, scaled_V);*/

    scale_epx(result_Y,height_down_Y,width_down_Y,scaled_Y,38);
    scale_epx(result_U,height_down_UV,width_down_UV,scaled_U,38);
    scale_epx(result_V,height_down_UV,width_down_UV,scaled_V,38);

    save_frame(output_filename, width_orig_Y, height_orig_Y, 1, scaled_Y,scaled_U,scaled_V,420);
    double psnr= get_PSNR_Y(scaled_Y,orig_Y, height_orig_Y,width_orig_Y);
    printf ("Coded file at: %s\n", input_filename);
    printf("PSNR: %f dB\n ",psnr);
    printf ("Discarded lines: %u\n", discarded_lines);
    printf ("Output file at: %s\n", output_filename);

    return 0;
}
