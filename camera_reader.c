/**
 * @file camera_reader.c
 * @author Francisco José Juan Quintanilla
 * @date July 2017
 * @brief Camera reader reads the camera and outputs the raw data.
 *
 * This module reads the the pi camera using the MMAL library from Raspberry Pi.
 * It outputs the data so the next module can encode the raw information into LHE.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */

#include "camera_reader.h"

#define MMAL_CAMERA_VIDEO_PORT 0

static MMAL_STATUS_T init_camera(CAMERA_OPTIONS *options, READY_CAMERA *camera)
{
  MMAL_COMPONENT_T *camera = 0;
  MMAL_ES_FORMAT_T *format;
  MMAL_PORT_T *video_port = NULL;
  MMAL_STATUS_T status;
  MMAL_POOL_T *pool;

  /* Create the component */
  status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

  if (status != MMAL_SUCCESS)
  {
     vcos_log_error("Failed to create camera component");
     goto error;
  }

  int camera_num = 0; // Sets the cammera number. As we will have only one it is set to 0

  status = mmal_port_parameter_set(camera->control, &camera_num);

  if (status != MMAL_SUCCESS)
  {
     vcos_log_error("Could not select camera : error %d", status);
     goto error;
  }

  if (!camera->output_num)
  {
     status = MMAL_ENOSYS;
     vcos_log_error("Camera doesn't have output ports");
     goto error;
  }

  status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG,1); // Mode is set to 1 @see https://www.raspberrypi.org/documentation/raspbian/applications/camera.md  for camera possible modes.

  if (status != MMAL_SUCCESS)
  {
     vcos_log_error("Could not set sensor mode : error %d", status);
     goto error;
  }

   video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];

   //  set up the camera configuration
   {
      MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
      {
         { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
         .max_stills_w = 1920,
         .max_stills_h = 1080,
         .stills_yuv422 = 0,
         .one_shot_stills = 0,
         .max_preview_video_w = 1920,
         .max_preview_video_h = 1080,
         .num_preview_video_frames = 3,
         .stills_capture_circular_buffer_height = 0,
         .fast_preview_resume = 0,
         .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
      };
      mmal_port_parameter_set(camera->control, &cam_config.hdr);
   }

   // Set the encode format on the video  port

   format = video_port->format;

   if(state->camera_parameters.shutter_speed > 6000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(video_port, &fps_range.hdr);
   }
   else if(state->camera_parameters.shutter_speed > 1000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 167, 1000 }, {999, 1000}};
        mmal_port_parameter_set(video_port, &fps_range.hdr);
   }


   format->encoding = MMAL_ENCODING_I420;
   format->encoding_variant = MMAL_ENCODING_I420;

   format->es->video.width = VCOS_ALIGN_UP(1920, 32);
   format->es->video.height = VCOS_ALIGN_UP(1080, 16);
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = 1920;
   format->es->video.crop.height = 1080;
   format->es->video.frame_rate.num = 30;
   format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

   status = mmal_port_format_commit(video_port);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera video format couldn't be set");
      goto error;
   }

   status = mmal_component_enable(camera);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera component couldn't be enabled");
      goto error;
   }

  return status;

error:

  if (camera)
     mmal_component_destroy(camera);

  return status;
}
