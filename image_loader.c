#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"
#include "include/imgUtil.h"

void init_image_loader (int width, int height)
{
    if (image_loader_initialized != true)
    {

        width_orig_Y = width;
        height_orig_Y = height;

        yuv_model = 0;
        height_orig_UV = height;
        width_orig_UV = width;

        orig_Y=malloc(height*sizeof (unsigned char *));
        orig_U=malloc(height*sizeof (unsigned char *));
        orig_V=malloc(height*sizeof (unsigned char *));

        for (int i=0;i<height_orig_Y;i++)
        {
            orig_Y[i]=malloc(width* sizeof (unsigned char));
            orig_U[i]=malloc(width* sizeof (unsigned char));
            orig_V[i]=malloc(width* sizeof (unsigned char));
        }
        image_loader_initialized = true;
    }
    else
    {
        fprintf(stderr,"%s:%s:%d:WARN: init_image_loader called but it was already initialized. Skkiping init\n", __FILE__,__func__ ,__LINE__, yuv_model);
    }
    return;
}

void init_image_loader_file (char const* filename)
{
    if (image_loader_initialized != true)
    {
        rgb=stbi_load(filename, &width_orig_Y, &height_orig_Y, &rgb_channels, 0);
        free(rgb);

        yuv_model = 0;
        height_orig_UV = height_orig_Y;
        width_orig_UV = width_orig_Y;

        orig_Y=malloc(height*sizeof (unsigned char *));
        orig_U=malloc(height*sizeof (unsigned char *));
        orig_V=malloc(height*sizeof (unsigned char *));

        for (int i=0;i<height_orig_Y;i++)
        {
            orig_Y[i]=malloc(width* sizeof (unsigned char));
            orig_U[i]=malloc(width* sizeof (unsigned char));
            orig_V[i]=malloc(width* sizeof (unsigned char));
        }
        image_loader_initialized = true;
    }
    else
    {
        fprintf(stderr,"%s:%s:%d:WARN: init_image_loader called but it was already initialized. Skkiping init\n", __FILE__,__func__ ,__LINE__, yuv_model);
    }
    return;
}


void load_image(char const* filename)
{
    if (DEBUG) fprintf(stderr,"%s:%s:%d:INFO: Loading frame %s \n", __FILE__,__func__ ,__LINE__, filename);

    rgb=stbi_load(filename, &width_orig_Y, &height_orig_Y, &rgb_channels, 0);

    if (image_loader_initialized == false)
    {
        init_image_loader(width_orig_Y, height_orig_Y);
    }

    rgb2yuv(rgb, rgb_channels);
    free(rgb);
}


void save_image(char const* filename, int width, int height, int channels, unsigned char **Y,unsigned char **U, unsigned char **V, int yuvmodel)
{
    if (DEBUG) fprintf(stderr,"%s:%s:%d:INFO: Saving frame %s \n", __FILE__,__func__ ,__LINE__, filename);

    unsigned char *data=malloc (channels*width*height);

    if (channels==1)
    {
        int pix=0;

        for (int line=0; line < height; line++)
        {
            for (int x=0; x<width; x++)
            {
                data[line*width+x]=Y[line][x];
            }
        }
    }
    else if (channels==3)
    {
        yuv2rgb(Y,U,V,3,width,height, data,yuvmodel);
    }
    int i = stbi_write_bmp(filename, width, height, channels, data);

    free(data);
    if (DEBUG) fprintf(stderr,"%s:%s:%d:INFO: Save frame result is %d \n", __FILE__,__func__ ,__LINE__, i);
}
