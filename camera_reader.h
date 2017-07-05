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
/*


struct RASPIVIDYUV_STATE_S
{
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int width;                          /// Requested width of image
   int height;                         /// requested height of image
   int framerate;                      /// Requested frame rate (fps)
   char *filename;                     /// filename of output file
   int verbose;                        /// !0 if want detailed run information
   int demoMode;                       /// Run app in demo mode
   int demoInterval;                   /// Interval between camera settings changes
   int waitMethod;                     /// Method for switching between pause and capture

   int onTime;                         /// In timed cycle mode, the amount of time the capture is on per cycle
   int offTime;                        /// In timed cycle mode, the amount of time the capture is off per cycle

   int onlyLuma;                       /// Only output the luma / Y plane of the YUV data
   int useRGB;                         /// Output RGB data rather than YUV

   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview

   MMAL_POOL_T *camera_pool;            /// Pointer to the pool of buffers used by camera video port

   PORT_USERDATA callback_data;         /// Used to move data to the camera callback

   int bCapturing;                      /// State of capture/pause

   int cameraNum;                       /// Camera number
   int settings;                        /// Request settings from the camera
   int sensor_mode;                     /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

   int frame;
   char *pts_filename;
   int save_pts;
   int64_t starttime;
   int64_t lasttime;

   bool netListen;
};

static MMAL_STATUS_T init_camera(CAMERA_OPTIONS *options, READY_CAMERA *camera);
int close_camera(camera_object);


*/
