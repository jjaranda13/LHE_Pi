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
#include "../include/imgUtil.h"
#include "../include/quantizer.h"
#include "../player/entropic_decoder.h"
#include "../player/quantizer_decoder.h"

int ratio_height_YUV;
int ratio_width_YUV;
int pppyUV;
int pppxUV;

int main(int argc, char* argv[])
{

    DEBUG = false;
    yuv_model = 2; // 4:2:0
    pppx = 2;
    pppy = 2;
    uint8_t * hops_obtained, *component_obtained;
    int tam_bits;
    hops_obtained = calloc(1000,sizeof(uint8_t));
    component_obtained = calloc(1000,sizeof(uint8_t));

    load_frame("../LHE_Pi/img/lena.bmp");
    rgb2yuv(rgb,rgb_channels);

    ratio_height_YUV=height_orig_Y/height_orig_UV;
    ratio_width_YUV=width_orig_Y/width_orig_UV;
    pppyUV=2*pppy/ratio_height_YUV;
    pppxUV=2*pppx/ratio_width_YUV;

    init_downsampler();
    init_quantizer();
    init_entropic_enc();

    for(int line =0 ; line< height_down_Y; line++) {
        //Coding
        down_avg_horiz(orig_Y,width_orig_Y,orig_down_Y,line,pppx,pppy);
        quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
        tam_bits =entropic_enc(hops_Y, bits_Y, line, width_down_Y);
        int tam_bytes = tam_bits%8 ? (tam_bits/8)+1: tam_bits/8;
        //Decoding
        int readed_bytes;
        int readed_symbols;

        readed_symbols = decode_symbols_entropic(bits_Y[line], hops_obtained, tam_bytes+20, width_down_Y, &readed_bytes);

        //Check
        if(readed_bytes != tam_bytes){
            printf("Error readed_bytes=%d is not equal to the tam_bytes=%d generated\n", readed_bytes, tam_bytes);
            break;
        }
        if(readed_symbols != width_down_Y){
            printf("Error readed_symbols=%d is not equal to the widtd=%d\n",readed_symbols, width_down_Y);
            break;
        }
        for (int i =0; i< width_down_Y; i++){
            if(hops_obtained[i] != hops_Y[line][i]){
                printf("Error in the symbol=%d line=%d\n", i, line);
                break;
            }
        }

        decode_line_quantizer(hops_obtained, component_obtained, readed_symbols);
        for (int i =0; i< width_down_Y; i++){
            if(component_obtained[i] != result_Y[line][i]){
                printf("Error in the x=%d line=%d component_obtained=%d result_Y=%d hop=%d\n", i, line,component_obtained[i],result_Y[line][i], hops_obtained[i]);
                break;
            }
        }
    }
    printf("frame right \n");
    return 0;
}
