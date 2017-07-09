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

 /**
  * Options to pass to the camera. All are mandatory.
  */
struct CAMERA_OPTIONS
{
  int width;                          /// Requested width of image
  int height;                         /// requested height of image
  int framerate;                      /// Requested frame rate (fps)
  int cameraNum;                       /// Camera number. Set it to 0 as default.
  int sensor_mode;                     /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.
}

/**
 * @brief Callback function that is called whne a frame is ready.
 *
 * The function to be called recieves a buffer with its indexes to obtain the
 * frame. A timestamp of the presentation time is obtained too.
 *
 * @p data Array of bytes contaning the image.
 * @p offset Offset of the start of the frame.
 * @p length Lengh of the payload. Counter after offset.
 * @p pts Presentation timestamp of the frame.
 */
typedef void(* ENCODE_CALLBACK) (uint8_t* data, uint32_t *offset, uint32_t length, int64_t pts);

/**
 * @brief Initiates the camera.
 *
 * This functions initites the camera and starts trwing frame to the callabck provided.
 *
 * @param options Options for ythe video capture.
 * @param cb callabck function to eb executed after a frame
 * @return MMAL_COMPONENT_T the camera component.
 */
static MMAL_COMPONENT_T init_camera(CAMERA_OPTIONS *options, ENCODE_CALLBACK cb);

/**
 * @brief Closes the camera passed.
 *
 *Stops and closes the camera passed.
 *
 * @param camera_object the camera to be closed.
 * @return Status of the process.
 */
int close_camera(camera_object);


// Private area
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

static void camera_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

typedef struct
{
   MMAL_POOL_T *pool;                   /// Pool in which the port has its buffer_headers
   ENCODE_CALLBACK cb;                 /// pointer to the callback
} PORT_USERDATA;
