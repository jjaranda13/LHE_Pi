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

#ifndef CAMERA_READER_H
#define CAMERA_READER_H

#include <interface/mmal/mmal.h>

 /**
  * @brief Prints the camera delay to the stderr.
  * 
  * This delay is the difference between the CSI interrruption. This 
  * corresponds to the time were the first line arrived at the GPU. and 
  * the time where the code actually adquired the image to process it.
  */
//#define PRINT_CAMERA_DELAY

 /**
  * @brief Prints the time between frames.
  * 
  */
//#define PRINT_PERIOD_FRAMES

 /**
  * @brief Camera Options.
  *
  * Options to pass to the camera. An instance of this struct must be send in
  * order to create the camera.
  */
typedef struct CAMERA_OPTIONS
{
  int width;                          /// Requested width of the image
  int height;                         /// requested height of the image
  int framerate;                      /// Requested frame rate (fps)
  int framerate_div;                      /// Skipping frames. Used to set a hight fps and then divide it by this number.
  int cameraNum;                      /// Camera number. Set it to 0 as default.
  int sensor_mode;                    /// Sensor mode. 0=auto. @see https://www.raspberrypi.org/documentation/raspbian/applications/camera.md
} CAMERA_OPTIONS;

/**
 * @brief Initiates the camera.
 *
 * This function initiates the camera and starts generating frames in the
 * global variables. It must be called before generating frames. It fills
 * height_orig_YUV width_orig_YUV. As soon it is inicialized it fills the
 * global variables orig_YUV with the data.
 *
 * It uses the global variable cam_down_sem in order to check whether the
 * previous frame is still in use. In case the previus buffer is still not
 * released the actual frame is dropped.
 *
 * @param options Options for ythe video capture.
 * @return The camera component created to handle the camera.
 */
MMAL_COMPONENT_T * init_camera(CAMERA_OPTIONS *options);

/**
 * @brief Closes the camera passed.
 *
 * Stops and closes the camera passed. Stops the new frames and allow the camera
 * to be used by other processes. It needs the object that was returned when the
 * camera was created.
 *
 * @param camera_object the camera to be closed.
 * @return Status of the process. 0=sucess 1=error
 */
int close_camera(MMAL_COMPONENT_T * camera_object);

#endif /* CAMERA_READER_H */
