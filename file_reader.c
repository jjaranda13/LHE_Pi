#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

#include "include/globals.h"
#include "include/imgUtil.h"

void init_image_loader (int width, int height)
{
    if (DEBUG) printf ("%s:%s:%d:DEBUG: Called with parameter (%d,%d)\n", __FILE__,__func__ ,__LINE__, width, height);
    
    if (image_loader_initialized == true)
    {
		fprintf(stderr,"%s:%s:%d:WARN: init_image_loader called but it was already initialized. Skkiping init\n", __FILE__,__func__ ,__LINE__, yuv_model);
		return;
	}

	width_orig_Y = width;
	height_orig_Y = height;

	yuv_model = YUV420;
	height_orig_UV = height/2;
	width_orig_UV = width/2;
	
	/* Enough memory to hold a YUV444. It is needed as in an
	 * intermediate step uses this memory. Finally it will hold a YUV420
	 */
	orig_Y = (unsigned char**)malloc(height_orig_Y * sizeof(unsigned char *));
	orig_U = (unsigned char**)malloc(height_orig_Y * sizeof(unsigned char *));
	orig_V = (unsigned char**)malloc(height_orig_Y * sizeof(unsigned char *));
	for (int i=0;i<height_orig_Y;i++)
	{
		orig_Y[i] = (unsigned char *)malloc(width_orig_Y* sizeof (unsigned char));
		orig_U[i] = (unsigned char *)malloc(width_orig_Y* sizeof (unsigned char));
		orig_V[i] = (unsigned char *)malloc(width_orig_Y* sizeof (unsigned char));
	}
  
	image_loader_initialized = true;
	if (DEBUG) printf ("%s:%s:%d:DEBUG: Finished -> width_orig_Y=%d height_orig_Y=%d width_orig_UV=%d height_orig_UV=%d  \n", __FILE__,__func__ ,__LINE__, width_orig_Y, height_orig_Y, height_orig_UV, width_orig_UV);
 
    return;
}

void init_image_loader_file (char filename[])
{
    if (DEBUG) printf ("%s:%s:%d:DEBUG: Called with parameter (%s)\n", __FILE__,__func__ ,__LINE__, filename);

    if (image_loader_initialized == true)
    {
        fprintf(stderr,"%s:%s:%d:WARN: init_image_loader called but it was already initialized. Skkiping init\n", __FILE__,__func__ ,__LINE__, yuv_model);
        return;
    }

    rgb=stbi_load(filename, &width_orig_Y, &height_orig_Y, &rgb_channels, 0);
	if (rgb == NULL)
    {
		fprintf(stderr,"%s:%s:%d:ERR: Specified file %s can not be loaded\n", __FILE__,__func__ ,__LINE__, filename);
    }
    free(rgb);
    
    init_image_loader (width_orig_Y, height_orig_Y);                    

    return;
}

int load_image(char filename[])
{
    if (DEBUG) fprintf(stderr,"%s:%s:%d:INFO: Loading frame %s \n", __FILE__,__func__ ,__LINE__, filename);

    if (image_loader_initialized == false)
    {
        init_image_loader_file(filename);
    }

    rgb=stbi_load(filename, &width_orig_Y, &height_orig_Y, &rgb_channels, 0);
    if (rgb == NULL)
    {
        return 1;
    }

    rgb2yuv(rgb, rgb_channels);
    free(rgb);

    yuv444toyuv420(orig_Y, orig_U, orig_V, width_orig_Y, height_orig_Y);

    return 0;
}

void save_image(char filename[], int width, int height, int channels, unsigned char **Y,unsigned char **U, unsigned char **V, int yuvmodel)
{
    int status;
    unsigned char * data;

    if (DEBUG) fprintf(stderr,"%s:%s:%d:INFO: Saving frame %s \n", __FILE__,__func__ ,__LINE__, filename);
    data = malloc(channels*width*height);
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

    status = stbi_write_bmp(filename, width, height, channels, data);
    if (status == 0)
    {
        fprintf(stderr,"%s:%s:%d:ERR: Could not write the specified %s file \n", __FILE__,__func__ ,__LINE__, filename);
    }

    free(data);
    if (DEBUG) fprintf(stderr,"%s:%s:%d:INFO: Save frame Ok\n", __FILE__,__func__ ,__LINE__);
}

