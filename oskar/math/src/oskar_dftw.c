/*
 * Copyright (c) 2017-2019, The University of Oxford
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

#include "math/define_dftw_c2c.h"
#include "math/define_dftw_o2c.h"
#include "math/define_dftw_m2m.h"
#include "math/define_multiply.h"
#include "math/oskar_dftw.h"
#include "utility/oskar_device.h"
#include "utility/oskar_kernel_macros.h"
#include "utility/oskar_vector_types.h"

#define DBL (1 << 0)
#define FLT (0 << 0)
#define D3  (1 << 1)
#define D2  (0 << 1)
#define DAT (1 << 2)
#define MAT (1 << 3)

OSKAR_DFTW_C2C_CPU(dftw_c2c_2d_float, 0, float, float2)
OSKAR_DFTW_C2C_CPU(dftw_c2c_3d_float, 1, float, float2)
OSKAR_DFTW_M2M_CPU(dftw_m2m_2d_float, 0, float, float2, float4c)
OSKAR_DFTW_M2M_CPU(dftw_m2m_3d_float, 1, float, float2, float4c)
OSKAR_DFTW_O2C_CPU(dftw_o2c_2d_float, 0, float, float2)
OSKAR_DFTW_O2C_CPU(dftw_o2c_3d_float, 1, float, float2)

OSKAR_DFTW_C2C_CPU(dftw_c2c_2d_double, 0, double, double2)
OSKAR_DFTW_C2C_CPU(dftw_c2c_3d_double, 1, double, double2)
OSKAR_DFTW_M2M_CPU(dftw_m2m_2d_double, 0, double, double2, double4c)
OSKAR_DFTW_M2M_CPU(dftw_m2m_3d_double, 1, double, double2, double4c)
OSKAR_DFTW_O2C_CPU(dftw_o2c_2d_double, 0, double, double2)
OSKAR_DFTW_O2C_CPU(dftw_o2c_3d_double, 1, double, double2)

static int get_block_size(int num_total)
{
    const int warp_size = 32;
    const int num_warps = (num_total + warp_size - 1) / warp_size;
    const int block_size = num_warps * warp_size;
    return ((block_size > 256) ? 256 : block_size);
}

void oskar_dftw(
        int normalise,
        int num_in,
        double wavenumber,
        const oskar_Mem* weights_in,
        const oskar_Mem* x_in,
        const oskar_Mem* y_in,
        const oskar_Mem* z_in,
        int offset_coord_out,
        int num_out,
        const oskar_Mem* x_out,
        const oskar_Mem* y_out,
        const oskar_Mem* z_out,
        const oskar_Mem* data,
        int offset_out,
        oskar_Mem* output,
        int* status)
{
    if (*status) return;
    const int location = oskar_mem_location(output);
    const int type = oskar_mem_precision(output);
    const int is_dbl = oskar_mem_is_double(output);
    const int is_3d = (z_in != NULL && z_out != NULL);
    const int is_data = (data != NULL);
    const int is_matrix = oskar_mem_is_matrix(output);
    if (!oskar_mem_is_complex(output) || !oskar_mem_is_complex(weights_in) ||
            oskar_mem_is_matrix(weights_in))
    {
        *status = OSKAR_ERR_BAD_DATA_TYPE;
        return;
    }
    if (oskar_mem_location(weights_in) != location ||
            oskar_mem_location(x_in) != location ||
            oskar_mem_location(y_in) != location ||
            oskar_mem_location(x_out) != location ||
            oskar_mem_location(y_out) != location)
    {
        *status = OSKAR_ERR_LOCATION_MISMATCH;
        return;
    }
    if (oskar_mem_precision(weights_in) != type ||
            oskar_mem_type(x_in) != type ||
            oskar_mem_type(y_in) != type ||
            oskar_mem_type(x_out) != type ||
            oskar_mem_type(y_out) != type)
    {
        *status = OSKAR_ERR_TYPE_MISMATCH;
        return;
    }
    if (is_data)
    {
        if (oskar_mem_location(data) != location)
        {
            *status = OSKAR_ERR_LOCATION_MISMATCH;
            return;
        }
        if (!oskar_mem_is_complex(data) ||
                oskar_mem_type(data) != oskar_mem_type(output) ||
                oskar_mem_precision(data) != type)
        {
            *status = OSKAR_ERR_TYPE_MISMATCH;
            return;
        }
    }
    if (is_3d)
    {
        if (oskar_mem_location(z_in) != location ||
                oskar_mem_location(z_out) != location)
        {
            *status = OSKAR_ERR_LOCATION_MISMATCH;
            return;
        }
        if (oskar_mem_type(z_in) != type || oskar_mem_type(z_out) != type)
        {
            *status = OSKAR_ERR_TYPE_MISMATCH;
            return;
        }
    }
    oskar_mem_ensure(output, (size_t) offset_out + num_out, status);
    if (*status) return;
    if (location == OSKAR_CPU)
    {
        if (is_data)
        {
            if (is_matrix)
            {
                if (is_3d)
                {
                    if (is_dbl)
                        dftw_m2m_3d_double(num_in, wavenumber,
                                oskar_mem_double2_const(weights_in, status),
                                oskar_mem_double_const(x_in, status),
                                oskar_mem_double_const(y_in, status),
                                oskar_mem_double_const(z_in, status),
                                offset_coord_out, num_out,
                                oskar_mem_double_const(x_out, status),
                                oskar_mem_double_const(y_out, status),
                                oskar_mem_double_const(z_out, status),
                                oskar_mem_double4c_const(data, status),
                                offset_out,
                                oskar_mem_double4c(output, status), 0);
                    else
                        dftw_m2m_3d_float(num_in, (float)wavenumber,
                                oskar_mem_float2_const(weights_in, status),
                                oskar_mem_float_const(x_in, status),
                                oskar_mem_float_const(y_in, status),
                                oskar_mem_float_const(z_in, status),
                                offset_coord_out, num_out,
                                oskar_mem_float_const(x_out, status),
                                oskar_mem_float_const(y_out, status),
                                oskar_mem_float_const(z_out, status),
                                oskar_mem_float4c_const(data, status),
                                offset_out,
                                oskar_mem_float4c(output, status), 0);
                }
                else
                {
                    if (is_dbl)
                        dftw_m2m_2d_double(num_in, wavenumber,
                                oskar_mem_double2_const(weights_in, status),
                                oskar_mem_double_const(x_in, status),
                                oskar_mem_double_const(y_in, status), 0,
                                offset_coord_out, num_out,
                                oskar_mem_double_const(x_out, status),
                                oskar_mem_double_const(y_out, status), 0,
                                oskar_mem_double4c_const(data, status),
                                offset_out,
                                oskar_mem_double4c(output, status), 0);
                    else
                        dftw_m2m_2d_float(num_in, (float)wavenumber,
                                oskar_mem_float2_const(weights_in, status),
                                oskar_mem_float_const(x_in, status),
                                oskar_mem_float_const(y_in, status), 0,
                                offset_coord_out, num_out,
                                oskar_mem_float_const(x_out, status),
                                oskar_mem_float_const(y_out, status), 0,
                                oskar_mem_float4c_const(data, status),
                                offset_out,
                                oskar_mem_float4c(output, status), 0);
                }
            }
            else
            {
                if (is_3d)
                {
                    if (is_dbl)
                        dftw_c2c_3d_double(num_in, wavenumber,
                                oskar_mem_double2_const(weights_in, status),
                                oskar_mem_double_const(x_in, status),
                                oskar_mem_double_const(y_in, status),
                                oskar_mem_double_const(z_in, status),
                                offset_coord_out, num_out,
                                oskar_mem_double_const(x_out, status),
                                oskar_mem_double_const(y_out, status),
                                oskar_mem_double_const(z_out, status),
                                oskar_mem_double2_const(data, status),
                                offset_out,
                                oskar_mem_double2(output, status), 0);
                    else
                        dftw_c2c_3d_float(num_in, (float)wavenumber,
                                oskar_mem_float2_const(weights_in, status),
                                oskar_mem_float_const(x_in, status),
                                oskar_mem_float_const(y_in, status),
                                oskar_mem_float_const(z_in, status),
                                offset_coord_out, num_out,
                                oskar_mem_float_const(x_out, status),
                                oskar_mem_float_const(y_out, status),
                                oskar_mem_float_const(z_out, status),
                                oskar_mem_float2_const(data, status),
                                offset_out,
                                oskar_mem_float2(output, status), 0);
                }
                else
                {
                    if (is_dbl)
                        dftw_c2c_2d_double(num_in, wavenumber,
                                oskar_mem_double2_const(weights_in, status),
                                oskar_mem_double_const(x_in, status),
                                oskar_mem_double_const(y_in, status), 0,
                                offset_coord_out, num_out,
                                oskar_mem_double_const(x_out, status),
                                oskar_mem_double_const(y_out, status), 0,
                                oskar_mem_double2_const(data, status),
                                offset_out,
                                oskar_mem_double2(output, status), 0);
                    else
                        dftw_c2c_2d_float(num_in, (float)wavenumber,
                                oskar_mem_float2_const(weights_in, status),
                                oskar_mem_float_const(x_in, status),
                                oskar_mem_float_const(y_in, status), 0,
                                offset_coord_out, num_out,
                                oskar_mem_float_const(x_out, status),
                                oskar_mem_float_const(y_out, status), 0,
                                oskar_mem_float2_const(data, status),
                                offset_out,
                                oskar_mem_float2(output, status), 0);
                }
            }
        }
        else
        {
            if (is_3d)
            {
                if (is_dbl)
                    dftw_o2c_3d_double(num_in, wavenumber,
                            oskar_mem_double2_const(weights_in, status),
                            oskar_mem_double_const(x_in, status),
                            oskar_mem_double_const(y_in, status),
                            oskar_mem_double_const(z_in, status),
                            offset_coord_out, num_out,
                            oskar_mem_double_const(x_out, status),
                            oskar_mem_double_const(y_out, status),
                            oskar_mem_double_const(z_out, status),
                            0, offset_out,
                            oskar_mem_double2(output, status), 0);
                else
                    dftw_o2c_3d_float(num_in, (float)wavenumber,
                            oskar_mem_float2_const(weights_in, status),
                            oskar_mem_float_const(x_in, status),
                            oskar_mem_float_const(y_in, status),
                            oskar_mem_float_const(z_in, status),
                            offset_coord_out, num_out,
                            oskar_mem_float_const(x_out, status),
                            oskar_mem_float_const(y_out, status),
                            oskar_mem_float_const(z_out, status),
                            0, offset_out,
                            oskar_mem_float2(output, status), 0);
            }
            else
            {
                if (is_dbl)
                    dftw_o2c_2d_double(num_in, wavenumber,
                            oskar_mem_double2_const(weights_in, status),
                            oskar_mem_double_const(x_in, status),
                            oskar_mem_double_const(y_in, status), 0,
                            offset_coord_out, num_out,
                            oskar_mem_double_const(x_out, status),
                            oskar_mem_double_const(y_out, status), 0,
                            0, offset_out,
                            oskar_mem_double2(output, status), 0);
                else
                    dftw_o2c_2d_float(num_in, (float)wavenumber,
                            oskar_mem_float2_const(weights_in, status),
                            oskar_mem_float_const(x_in, status),
                            oskar_mem_float_const(y_in, status), 0,
                            offset_coord_out, num_out,
                            oskar_mem_float_const(x_out, status),
                            oskar_mem_float_const(y_out, status), 0,
                            0, offset_out,
                            oskar_mem_float2(output, status), 0);
            }
        }
    }
    else
    {
        size_t local_size[] = {256, 1, 1}, global_size[] = {1, 1, 1};
        const void* np = 0;
        const char* k = 0;
        int max_in_chunk;
        float wavenumber_f = (float) wavenumber;

        /* Select the kernel. */
        switch (is_dbl * DBL | is_3d * D3 | is_data * DAT | is_matrix * MAT)
        {
        case D2 | FLT:             k = "dftw_o2c_2d_float";  break;
        case D2 | DBL:             k = "dftw_o2c_2d_double"; break;
        case D3 | FLT:             k = "dftw_o2c_3d_float";  break;
        case D3 | DBL:             k = "dftw_o2c_3d_double"; break;
        case D2 | FLT | DAT:       k = "dftw_c2c_2d_float";  break;
        case D2 | DBL | DAT:       k = "dftw_c2c_2d_double"; break;
        case D3 | FLT | DAT:       k = "dftw_c2c_3d_float";  break;
        case D3 | DBL | DAT:       k = "dftw_c2c_3d_double"; break;
        case D2 | FLT | DAT | MAT: k = "dftw_m2m_2d_float";  break;
        case D2 | DBL | DAT | MAT: k = "dftw_m2m_2d_double"; break;
        case D3 | FLT | DAT | MAT: k = "dftw_m2m_3d_float";  break;
        case D3 | DBL | DAT | MAT: k = "dftw_m2m_3d_double"; break;
        default:
            *status = OSKAR_ERR_FUNCTION_NOT_AVAILABLE;
            return;
        }
        if (oskar_device_is_nv(location))
            local_size[0] = (size_t) get_block_size(num_out);
        oskar_device_check_local_size(location, 0, local_size);
        global_size[0] = oskar_device_global_size(
                (size_t) num_out, local_size[0]);

        /* max_in_chunk must be multiple of 16. */
        max_in_chunk = is_3d ? (is_dbl ? 384 : 800) : (is_dbl ? 448 : 896);
        if (is_data && is_3d && !is_dbl) max_in_chunk = 768;
        const size_t element_size = is_dbl ? sizeof(double) : sizeof(float);
        const size_t local_mem_size = max_in_chunk * element_size;
        const size_t arg_size_local[] = {
                2 * local_mem_size, 2 * local_mem_size,
                (is_3d ? local_mem_size : 0)
        };

        /* Set kernel arguments. */
        const oskar_Arg args[] = {
                {INT_SZ, &num_in},
                {is_dbl ? DBL_SZ : FLT_SZ,
                        is_dbl ? (void*)&wavenumber : (void*)&wavenumber_f},
                {PTR_SZ, oskar_mem_buffer_const(weights_in)},
                {PTR_SZ, oskar_mem_buffer_const(x_in)},
                {PTR_SZ, oskar_mem_buffer_const(y_in)},
                {PTR_SZ, is_3d ? oskar_mem_buffer_const(z_in) : &np},
                {INT_SZ, &offset_coord_out},
                {INT_SZ, &num_out},
                {PTR_SZ, oskar_mem_buffer_const(x_out)},
                {PTR_SZ, oskar_mem_buffer_const(y_out)},
                {PTR_SZ, is_3d ? oskar_mem_buffer_const(z_out) : &np},
                {PTR_SZ, is_data ? oskar_mem_buffer_const(data) : &np},
                {INT_SZ, &offset_out},
                {PTR_SZ, oskar_mem_buffer(output)},
                {INT_SZ, &max_in_chunk}
        };
        oskar_device_launch_kernel(k, location, 1, local_size, global_size,
                sizeof(args) / sizeof(oskar_Arg), args,
                sizeof(arg_size_local) / sizeof(size_t), arg_size_local,
                status);
    }
    if (normalise)
        oskar_mem_scale_real(output, 1. / num_in, offset_out, num_out, status);
}
