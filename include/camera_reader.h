/**
 * @file camera_reader.h
 * @author Francisco José Juan Quintanilla
 * @date July 2017
 * @brief Camera reader reads the camera and outputs YUV data.
 *
 * This module reads the the pi camera using the MMAL library from Broadcom. It
 * sends the video in YUV format to a global variable that is readed further
 * in the downsampler.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <interface/mmal/mmal.h>
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"

#include "globals.h"

 /**
  * Options to pass to the camera. All are mandatory.
  */
typedef struct CAMERA_OPTIONS
{
  int width;                          /// Requested width of the image
  int height;                         /// requested height of the image
  int framerate;                      /// Requested frame rate (fps)
  int cameraNum;                      /// Camera number. Set it to 0 as default.
  int sensor_mode;                    /// Sensor mode. 0=auto. @see https://www.raspberrypi.org/documentation/raspbian/applications/camera.md
} CAMERA_OPTIONS;



/**
 * @brief Initiates the camera.
 *
 * This function initiates the camera and starts generating frames in the global variables.
 * It must be called at first and generated frames asap.
 *
 * @param options Options for ythe video capture.
 * @return The camera component created to handle the camera.
 */
MMAL_COMPONENT_T * init_camera(CAMERA_OPTIONS *options);

/**
 * @brief Closes the camera passed.
 *
 * Stops and closes the camera passed. Stops the new frames and allow the camera to be used by other processes.
 *
 * @param camera_object the camera to be closed.
 * @return Status of the process. 0=sucess 1=error
 */
int close_camera(MMAL_COMPONENT_T * camera_object);


// Private area
void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

void camera_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

typedef struct
{
   MMAL_POOL_T *pool ;                    // Pool which contains the buffers we are using.
   MMAL_BUFFER_HEADER_T *previous_buffer; // Buffer of the previous frame. Passed in order to release it.
} PORT_USERDATA;
