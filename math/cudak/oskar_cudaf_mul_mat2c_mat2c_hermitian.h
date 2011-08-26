/*
 * Copyright (c) 2011, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSKAR_CUDAF_MUL_MAT2C_MAT2C_HERMITIAN_H_
#define OSKAR_CUDAF_MUL_MAT2C_MAT2C_HERMITIAN_H_

/**
 * @file oskar_cudaf_mul_mat2c_mat2c_hermitian.h
 */

#include "utility/oskar_cuda_eclipse.h"
#include "math/cudak/oskar_cudaf_mul_c_c.h"

/**
 * @brief
 * CUDA device function to multiply two complex 2x2 matrices (single precision).
 *
 * @details
 * This inline device function multiplies together two complex 2x2 matrices.
 * The Hermitian conjugate of the second matrix is taken before the
 * multiplication.
 *
 * Matrix multiplication is done in the order M = M1 * M2^H.
 *
 * @param[in] m1 The first complex matrix.
 * @param[in] m2 The second complex matrix.
 * @param[out] m The output complex number.
 */
__device__ __forceinline__ void oskar_cudaf_mul_mat2c_mat2c_hermitian_f(
        const float4c& m1, const float4c& m2, float4c& m)
{
}

/**
 * @brief
 * CUDA device function to multiply two complex 2x2 matrices (double precision).
 *
 * @details
 * This inline device function multiplies together two complex 2x2 matrices.
 * The Hermitian conjugate of the second matrix is taken before the
 * multiplication.
 *
 * Matrix multiplication is done in the order M = M1 * M2^H.
 *
 * @param[in] m1 The first complex matrix.
 * @param[in] m2 The second complex matrix.
 * @param[out] m The output complex number.
 */
__device__ __forceinline__ void oskar_cudaf_mul_mat2c_mat2c_hermitian_d(
        const double4c& m1, const double4c& m2, double4c& m)
{
}

#endif // OSKAR_CUDAF_MUL_MAT2C_MAT2C_HERMITIAN_H_