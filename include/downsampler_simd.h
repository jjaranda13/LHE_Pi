/**
 * @brief Downsamples 32 unsigned bytes by a factor of 2.
 *
 * It takes 2 pointers to data, it uses the first one to take 32 bytes and
 * writes into the destination 16 m bytes as a result of the interpolation as averaging.

 * @param orig Pointer to the data to be downsampled.
 * @param dest Pointer to store the subsampled data.
 *
 * Internally it is written in neon assembly for ARM.
 */
extern void _downsample_by2_simd( unsigned char *orig, unsigned char *dest);
