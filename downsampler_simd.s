.section .text

/**
 * The input data is:
 * r0 -> Pointer to the source data.
 * r1 -> Pointer to tstore the resulted data.
 *
 * It does not return.
 *
 * Pseudo algorithm:
 *  - Load the data interleving. Up to fill 2 q registers(32 bytes of data loaded).
 *  - Order the data in order to process as a q register.
 *  - Add and halve(vhadd ftw) the data and the shifted data.
 *  - Store the result from the previous operation interleaving.
 */
.globl _downsample_by2_simd
.func  _downsample_by2_simd
_downsample_by2_simd:

    vld2.8      {d0,d1}, [r0]!
    vld2.8      {d2,d3}, [r0]
    vswp        d1, d2

    vhadd.u8    q0, q0, q1

    vst1.8      d0, [r1]!
    vst1.8      d1, [r1]

    bx       lr

.endfunc
