/**
 * @file camera_reader.h
 * @author Francisco José Juan Quintanilla
 * @date July 2017
 * @brief Camera reader reads the camera and outputs the raw data.
 *
 * This module reads the the pi camera using the MMAL library from Raspberry Pi.
 * It outputs the data so the next module can encode the raw information into LHE.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */

struct CAMERA_OPTIONS
{
  int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
  int width;                          /// Requested width of image
  int height;                         /// requested height of image
  int framerate;                      /// Requested frame rate (fps)
  int verbose;                        /// !0 if want detailed run information
  int cameraNum;                       /// Camera number. Set it to 0 as default.
  int sensor_mode;                     /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.
}

struct CAMERA_BUFFER
{
  MMAL_COMPONENT_T camera_component;
  MMAL_BUFFER_HEADER_T buffer_header;
}

typedef void(* ENCODE_CALLBACK) (uint8_t* data, uint32_t *offset, uint32_t length, int64_t pts);

static MMAL_COMPONENT_T init_camera(CAMERA_OPTIONS *options, ENCODE_CALLBACK cb);

static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

static void camera_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

int close_camera(camera_object);
