/**
 * @file camera_reader.c
 * @author Francisco José Juan Quintanilla
 * @date July 2017
 * @brief Camera reader reads the camera and outputs YUV data.
 *
 * This module reads the the pi camera using the MMAL library from Broadcom. It
 * sends the video in YUV format to a global variable that is readed further.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */

#include "camera_reader.h"

#define MMAL_CAMERA_VIDEO_PORT 0
#define VIDEO_FRAME_RATE_DEN 1
#define VIDEO_OUTPUT_BUFFERS_NUM 5

MMAL_COMPONENT_T * init_camera(CAMERA_OPTIONS *options)
{
    MMAL_COMPONENT_T *camera = 0;
    MMAL_ES_FORMAT_T *format;
    MMAL_PORT_T *video_port = NULL;
    MMAL_STATUS_T status;
    MMAL_POOL_T *pool= 0;
    PORT_USERDATA * callback_data = (PORT_USERDATA * ) malloc(sizeof(PORT_USERDATA));

    if (DEBUG) printf ("%s:%s:%d:DEBUG: Called init_camera\n", __FILE__,__func__ ,__LINE__);

    //Parse options into global variables to be used between processes.

    height_orig_Y = options->height;
    width_orig_Y = options->width;

    if (yuv_model)
    {
        if (yuv_model == 0)
        {
            height_orig_UV = options->height;
            width_orig_UV = options->width;
        }
        else if (yuv_model == 1)
        {
            height_orig_UV = options->height;
            width_orig_UV = options->width>>1;
        }
        else if (yuv_model == 2)
        {
            height_orig_UV = options->height>>1;
            width_orig_UV = options->width>>1;
        }
        else
        {
            fprintf(stderr,"%s:%s:%d:ERROR: Unknown yuv_model -> yuv_model = %d when if should be in range 0-2.\n", __FILE__,__func__ ,__LINE__, yuv_model);
            goto error;
        }
        //Setting Globals allocation
        orig_Y = (unsigned char**) malloc(sizeof(unsigned char *) *height_orig_Y);
        orig_U = (unsigned char**) malloc(sizeof(unsigned char *) *height_orig_UV);
        orig_V = (unsigned char**) malloc(sizeof(unsigned char *) *height_orig_UV);
        cam_down_sem = 0;
    }
    else
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Unset yuv_model -> yuv_model must be setted before calling init_camera.\n", __FILE__,__func__ ,__LINE__);
        goto error;
    }

    if (DEBUG) printf ("%s:%s:%d:DEBUG: Setted height and weight globals\n", __FILE__,__func__ ,__LINE__);

    // Create the camera component
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr, "%s:%s:%d:ERROR: Error creating camera component\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    }
    else if (DEBUG) printf ("%s:%s:%d:DEBUG: Camera created sucessfully\n", __FILE__,__func__ ,__LINE__);

    // Select the Camera from options
    MMAL_PARAMETER_INT32_T camera_num = {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, options->cameraNum};

    status = mmal_port_parameter_set(camera->control, &camera_num.hdr);

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Could not select camera\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    }
    else if (DEBUG) printf ("%s:%s:%d:DEBUG: Camera number %d selected sucessfully\n", __FILE__,__func__ ,__LINE__, options->cameraNum);

    // Check that the camera has enough outputs
    if (!camera->output_num)
    {
        status = MMAL_ENOSYS;
        fprintf(stderr,"%s:%s:%d:ERROR: Camera doesn't have output ports\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    }


    // Set Mode of the camera
    status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG,options->sensor_mode); // Mode is set to 1 @see https://www.raspberrypi.org/documentation/raspbian/applications/camera.md  for camera possible modes.

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Could not set sensor mode\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    }
    else if (DEBUG) printf ("%s:%s:%d:DEBUG: Camera mode %d selected sucessfully\n", __FILE__,__func__ ,__LINE__, options->sensor_mode);

    MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
    {
        { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
        .max_stills_w = options->width,
        .max_stills_h = options->height,
        .stills_yuv422 = 0,
        .one_shot_stills = 0,
        .max_preview_video_w = options->width,
        .max_preview_video_h = options->height,
        .num_preview_video_frames = 3,
        .stills_capture_circular_buffer_height = 0,
        .fast_preview_resume = 0,
        .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
    };
    status = mmal_port_parameter_set(camera->control, &cam_config.hdr);

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Could not set camera config\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    }
    else if (DEBUG) printf ("%s:%s:%d:DEBUG: Camera extra parameters configured sucessfully\n", __FILE__,__func__ ,__LINE__);


  // TODO-> Much more parameters can be set on the control port in order to alterate the camera behaviour. Explore those in the future.

  // As we finished with the control port we have to enable it giving it a callback function for events generated and stuff.
    status = mmal_port_enable(camera->control, camera_control_callback);

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Unable to enable control port\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    }
    else if (DEBUG) printf ("%s:%s:%d:DEBUG: Control port enabled sucessfully\n", __FILE__,__func__ ,__LINE__);

    // Obtain the output port from the camera to configure it.
    video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];

    format = video_port->format;

    format->encoding = MMAL_ENCODING_I420; // @See https://www.fourcc.org/pixel-format/yuv-i420/
    format->es->video.width = VCOS_ALIGN_UP(options->width, 32);
    format->es->video.height = VCOS_ALIGN_UP(options->height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = options->width;
    format->es->video.crop.height = options->height;
    format->es->video.frame_rate.num = options->framerate;
    format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

    status = mmal_port_format_commit(video_port);

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Camera video format couldn't be set\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    } else if (DEBUG) printf ("%s:%s:%d:DEBUG: Video port configured sucessfully\n", __FILE__,__func__ ,__LINE__);

    // Ensure there are enough buffers to avoid dropping frames
    if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
        video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;


    status = mmal_component_enable(camera);

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Camera component couldn't be enabled\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    } else if (DEBUG) printf ("%s:%s:%d:DEBUG: Camera component enabled \n", __FILE__,__func__ ,__LINE__);

    status = mmal_port_parameter_set_boolean(video_port, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Failed to select zero copy\nError -> %d\nStatus -> %s", __FILE__,__func__ ,__LINE__, status, mmal_status_to_string(status));
        goto error;
    } else if (DEBUG) printf ("%s:%s:%d:DEBUG: Zero Copy enabled for the video port \n", __FILE__,__func__ ,__LINE__);

    /* Create pool of buffer headers for the output port to consume */
    pool = mmal_port_pool_create(video_port, video_port->buffer_num, video_port->buffer_size);

    if (!pool)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Failed to create buffer header pool for camera still port %s", __FILE__,__func__ ,__LINE__, video_port->name);
    } else if (DEBUG) printf ("%s:%s:%d:DEBUG: Pool for the buffers is created \n", __FILE__,__func__ ,__LINE__);

    callback_data->pool = pool;
    callback_data->previous_buffer= NULL;
    video_port->userdata = (struct MMAL_PORT_USERDATA_T *)callback_data;

    if (DEBUG) printf ("%s:%s:%d:DEBUG: Callback data introduced into the output port \n", __FILE__,__func__ ,__LINE__);

    status = mmal_port_enable(video_port, camera_buffer_callback);

    if (status != MMAL_SUCCESS)
    {
        fprintf(stderr,"%s:%s:%d:ERROR: Failed to enable the camera output port\n", __FILE__,__func__ ,__LINE__);
        goto error;
    } else if (DEBUG) printf ("%s:%s:%d:DEBUG: Output port of the camera enabled \n", __FILE__,__func__ ,__LINE__);

    {
        int num = mmal_queue_length(pool->queue);
        int q;
        for (q=0;q<num-1;q++)
        {
            MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);

            if (!buffer)
                fprintf(stderr,"%s:%s:%d:ERROR: Unable to get a required buffer %d from pool queue\n", __FILE__,__func__ ,__LINE__, q);

            status = mmal_port_send_buffer(video_port, buffer);
            if (status!= MMAL_SUCCESS)
                fprintf(stderr,"%s:%s:%d:ERROR: Unable to send a buffer to camera video port (%d), %s\n", __FILE__,__func__ ,__LINE__, q, mmal_status_to_string(status));
        }
    }
    if (DEBUG) printf ("%s:%s:%d:DEBUG: Port filled with buffers\n", __FILE__,__func__ ,__LINE__);

    printf ("%s:%s:%d:INFO: Camera Started\n", __FILE__,__func__ ,__LINE__);

    return camera;

error:

  if (camera)
    mmal_component_destroy(camera);
  if (pool)
    mmal_pool_destroy(pool);
  return 0;
}


void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    // We do not really need to do anything so just release the buffer
    if (DEBUG) printf ("%s:%s:%d:DEBUG: camera_control_callback: Called camera_control_callback\n", __FILE__,__func__ ,__LINE__);
    mmal_buffer_header_release(buffer);
}


void camera_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    //if (DEBUG) printf ("DEBUG:camera_buffer_callback: Called camera_buffer_callback\n");
    PORT_USERDATA *callback_data;
    callback_data = (PORT_USERDATA *)port->userdata;


    if (callback_data->previous_buffer) // release the previous buffer back to the pool
    {
        mmal_buffer_header_release(callback_data->previous_buffer);
    }

    if (cam_down_sem == 0) // Downsampler finished its work so the global pointers can b modified.
    {

        int index = 0;
        int y=0;

        for (y=0; y<height_orig_Y; y++)
        {
            orig_Y[y]= buffer->data + index;
            index += width_orig_Y;
        }
        for (y=0; y<height_orig_UV; y++)
        {
            orig_U[y]= buffer->data + index;
            index += width_orig_UV;
        }
        for (y=0; y<height_orig_UV; y++)
        {
            orig_V[y]= buffer->data + index;
            index += width_orig_UV;
        }

        cam_down_sem = 1;
        callback_data->previous_buffer = buffer;
        if (DEBUG) printf("%s:%s:%d:DEBUG: Processed frame\n", __FILE__,__func__ ,__LINE__);

    } else {

        // Do not copy the image and drop the frame
        callback_data->previous_buffer = NULL;
        mmal_buffer_header_release(buffer);
        printf("%s:%s:%d:INFO: Dropped frame\n", __FILE__,__func__ ,__LINE__);
    }

    // and send one back to the port (if still open)
    if (port->is_enabled)
    {
        //printf("Sending a buffer\n");
        MMAL_STATUS_T status;
        MMAL_BUFFER_HEADER_T *new_buffer;


        new_buffer = mmal_queue_get(callback_data->pool->queue);

        if (new_buffer)
        {
            status = mmal_port_send_buffer(port, new_buffer);
        }
        else
        {
            fprintf(stderr,"%s:%s:%d:ERROR: Unable to obtain a buffer\n", __FILE__,__func__ ,__LINE__);
        }
        if (!new_buffer || status != MMAL_SUCCESS)
        {
            fprintf(stderr,"%s:%s:%d:ERROR: Unable to return a buffer to the camera port\n", __FILE__,__func__ ,__LINE__);
        }
    }
}


int close_camera(MMAL_COMPONENT_T * camera_object)
{

    if (camera_object)
    {
        mmal_component_destroy(camera_object);
        printf ("%s:%s:%d:INFO: Camera Closed\n", __FILE__,__func__ ,__LINE__);
        return 0;
    }

    return 1;
}


