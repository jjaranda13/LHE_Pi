/**
 * @brief Computes the delta sum in SIMD.
 *
 * It takes the pointers for the last frame, the delta scan and the destination
 * and applies the conversion to the delta domain transformating the signal. It
 * mimics the behaviour of compute_delta_scanline in loop code. Internally it
 * is written in neon assembly for ARM.It process eitght uint8_t in each run,
 * beaware of it
 *
 * @param orig Pointer to the original actual frame data
 * @param last_frame_player Pointer to the past frame data
 * @param delta Pointer to the location to store the delta result.
 *
 */
inline void _compute_delta_simd( unsigned char * orig, unsigned char * last_frame_player, unsigned char *delta);
