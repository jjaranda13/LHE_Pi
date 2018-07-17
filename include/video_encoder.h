
void compute_delta(unsigned char **frame_new,unsigned char **frame_prev, int y,int width, unsigned char **delta);

/**
 * @brief Computes the delta diference image for a particular line.
 *
 * This function performs the transformation into the differential domain. It
 * uses the actual frame and the previous frame. It only computes it for the
 * indicated line. It si the same as compute_delta_scanline but the calculation
 * is done in NEON vector unit.
 *
 * @param y Scanline to be processed
 * @param width width of the image to be processed.
 * @param orig_down Actual image, used for the delta.
 * @param last_frame_player Previous image, used for the delta.
 * @param delta The result of the operation is left in this pointer
 * @return void
 */
void compute_delta_scanline_simd(int y, int width, unsigned char ** orig_down, unsigned char ** last_frame_player, unsigned char ** delta);

void init_videoencoder();
