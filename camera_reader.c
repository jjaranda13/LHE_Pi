/**
 * @file camera_reader.c
 * @author Francisco José Juan Quintanilla
 * @date July 2017
 * @brief Camera reader reads the camera and outputs the raw data.
 *
 * This module reads the the pi camera using the MMAL library from Broadcom. It
 * sends the video in raw to be further processed.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */

#include "camera_reader.h"

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"

#define MMAL_CAMERA_VIDEO_PORT 0
#define VIDEO_FRAME_RATE_DEN 1
#define VIDEO_OUTPUT_BUFFERS_NUM 1

static MMAL_COMPONENT_T init_camera(CAMERA_OPTIONS *options, ENCODE_CALLBACK cb)
{
  MMAL_COMPONENT_T *camera = 0;
  MMAL_ES_FORMAT_T *format;
  MMAL_PORT_T *video_port = NULL;
  MMAL_STATUS_T status;
  MMAL_POOL_T *pool= 0;

  // Create the camera component
  status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

  if (status != MMAL_SUCCESS)
  {
     fprintf(stderr, "Error creating camera component\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
     goto error;
  }
  // Select the Camera from options
  MMAL_PARAMETER_INT32_T camera_num =
     {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, options->cameraNum};

  status = mmal_port_parameter_set(camera->control, &camera_num.hdr);

  if (status != MMAL_SUCCESS)
  {
     fprintf(stderr,"Could not select camera\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
     goto error;
  }
  // Check that the camera has enough outputs
  if (!camera->output_num)
  {
     status = MMAL_ENOSYS;
     fprintf(stderr,"Camera doesn't have output ports\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
     goto error;
  }
  // Set Mode of the camera
  status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG,options->sensor_mode); // Mode is set to 1 @see https://www.raspberrypi.org/documentation/raspbian/applications/camera.md  for camera possible modes.

  if (status != MMAL_SUCCESS)
  {
     fprintf(stderr,"Could not set sensor mode\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
     goto error;
  }

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
     fprintf(stderr,"Could not set camera config\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
     goto error;
  }

  // TODO-> Much more parameters can be set on the control port in order to alterate the camera behaviour. Explore those in the future.

  // As we finished with the control port we have to enable it giving it a callback function for events generated and stuff.
  status = mmal_port_enable(camera->control, camera_control_callback);

  if (status != MMAL_SUCCESS)
  {
     fprintf(stderr,"Unable to enable control port\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
     goto error;
  }

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
      fprintf(stderr,"camera video format couldn't be set\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
      goto error;
   }

   status = mmal_component_enable(camera);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera component couldn't be enabled\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
      goto error;
   }

   // Ensure there are enough buffers to avoid dropping frames
   if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   status = mmal_port_parameter_set_boolean(video_port, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Failed to select zero copy\nError -> %d\nStatus -> %s", status, mmal_status_to_string(status));
      goto error;
   }

   /* Create pool of buffer headers for the output port to consume */
   pool = mmal_port_pool_create(video_port, video_port->buffer_num, video_port->buffer_size);

   if (!pool)
   {
      vcos_log_error("Failed to create buffer header pool for camera still port %s", video_port->name);
   }
   PORT_USERDATA callback_data;
   callback_data->pool = pool;
   callback_data->cb= &cb
   camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *)&callback_data;

   // Send all the buffers to the camera video port
   {
      int num = mmal_queue_length(pool->queue);
      int q;
      for (q=0;q<num;q++)
      {
         MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);

         if (!buffer)
            vcos_log_error("Unable to get a required buffer %d from pool queue", q);

         if (mmal_port_send_buffer(camera_video_port, buffer)!= MMAL_SUCCESS)
            vcos_log_error("Unable to send a buffer to camera video port (%d)", q);
      }
   }

   status = mmal_port_enable(camera_video_port, camera_buffer_callback);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Failed to setup camera output");
      goto error;
   }

  return camera;

error:

  if (camera)
    mmal_component_destroy(camera);
  if (pool)
    mmal_pool_destroy(pool);
  return 0;
}


static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   // We do not really need to do anything so just release the buffer
   mmal_buffer_header_release(buffer);
}


static void camera_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
  // TODO -> I need the pool in the userdata
  port->userdata->cb(buffer->data,buffer->offset, buffer->length,buffer->pts);

  // release buffer back to the pool
  mmal_buffer_header_release(buffer);

  // and send one back to the port (if still open)
  if (port->is_enabled)
  {
     MMAL_STATUS_T status;
     MMAL_BUFFER_HEADER_T *new_buffer;

     new_buffer = mmal_queue_get(port->userdata->pool->queue);

     if (new_buffer)
        status = mmal_port_send_buffer(port, new_buffer);

     if (!new_buffer || status != MMAL_SUCCESS)
        vcos_log_error("Unable to return a buffer to the camera port");
  }
}


int close_camera(camera_object)
{
  if (camera_object)
     mmal_component_destroy(camera_object);
  return 0;
}
