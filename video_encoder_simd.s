.section .text

/**
 * The input data is:
 * r0 -> Pointer to the orig frame data.
 * r1 -> Pointer to the past frame data.
 * r2 -> Pointer to the delta destination lo leave the delta.
 *
 * It does not return.
 *
 * Pseudo algorithm:
 *  - Load the data from the actual frame.
 *  - Load the data from the previous frame.
 *  - Sub the orig minus the previous. Ops are in uint8 result should be int16.
 *  - Store the sign in a register and take the absolute value of the components.
 *  - If the delta is bigger than tramo 2 truncate it to tramo2-1
 *  - Now compare with range 1 and compute the transformation
 *  - Write back the result of the computation where it was true
 *  - Sum the result to 128, beaware of the sign
 *  - Transform to uint8_t and save it back to memory.
 *
 * Registers:
 *  - q0 = Difference between frames, delta
 *  - q1 = Sign
 *  - q2 = Rango 1, Rango 2, Rango 2 -1, #128
 *  - q3 = Result for operations that take place in the ifs
 *  - q4 = Mask for the comparations of the intermadiate ifs
 *
 */
.globl _compute_delta_simd
.func  _compute_delta_simd
_compute_delta_simd:

    vld1.8          d0, [r0]
    vld1.8          d1, [r1]



    vsubl.u8        q0, d0, d1  // Difference between frame is stored  as int16
    vclt.s16        q1, q0, #0  // q1 will store the sign
    vqabs.s16       q0, q0

    vmov.i16        q2, #204    // Tramo 2 constant is loaded
    vcge.s16        q4, q0, q2
    vmov.i16        q2, #203    // Tramo 2-1
    vbit            q0, q2, q4  // If delta is bigger than Tramo 2 it is set to tramo2-1

    vmov.i16        q2, #52     // Tramo1
    vcge.s16        q4, q0, q2
    vqsub.s16       q3, q0, q2
    vshr.s16        q3, q3, #1 // Divide by 2.
    vadd.s16        q3, q3, q2
    vbit            q0, q3, q4

    vqneg.s16       q3, q0
    vbit            q0, q3, q1  // Copy only the ones taht were negative.
    vmov.i16        q2, #128    // 128 -> Half of the range
    vadd.s16        q0, q0, q2

    vqmovun.s16     d0, q0
    vst1.8          d0, [r2]

    bx       lr

.endfunc
