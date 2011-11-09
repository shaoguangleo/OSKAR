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

#include "interferometry/oskar_telescope_model_scale_coords.h"
#include "station/oskar_station_model_scale_coords.h"
#include "math/cudak/oskar_cudak_vec_scale_rr.h"
#include <cuda_runtime_api.h>

#ifdef __cplusplus
extern "C" {
#endif

int oskar_telescope_model_scale_coords(oskar_TelescopeModel* telescope,
        double value)
{
    if (telescope == NULL)
        return OSKAR_ERR_INVALID_ARGUMENT;

    if (telescope->location() != OSKAR_LOCATION_GPU)
        return OSKAR_ERR_BAD_LOCATION;

    int num_antennas = telescope->num_stations;
    int num_threads = 256;
    int num_blocks  = (num_antennas + num_threads - 1) / num_threads;

    // Scale the telescope coordinates.
    if (telescope->type() == OSKAR_DOUBLE)
    {
        oskar_cudak_vec_scale_rr_d OSKAR_CUDAK_CONF(num_blocks, num_threads)
            (num_antennas, value, telescope->station_x);
        oskar_cudak_vec_scale_rr_d OSKAR_CUDAK_CONF(num_blocks, num_threads)
            (num_antennas, value, telescope->station_y);
        oskar_cudak_vec_scale_rr_d OSKAR_CUDAK_CONF(num_blocks, num_threads)
            (num_antennas, value, telescope->station_z);
    }
    else if (telescope->type() == OSKAR_SINGLE)
    {
        oskar_cudak_vec_scale_rr_f OSKAR_CUDAK_CONF(num_blocks, num_threads)
            (num_antennas, (float)value, telescope->station_x);
        oskar_cudak_vec_scale_rr_f OSKAR_CUDAK_CONF(num_blocks, num_threads)
            (num_antennas, (float)value, telescope->station_y);
        oskar_cudak_vec_scale_rr_f OSKAR_CUDAK_CONF(num_blocks, num_threads)
            (num_antennas, (float)value, telescope->station_z);
    }
    else
    {
        return OSKAR_ERR_BAD_DATA_TYPE;
    }

    cudaDeviceSynchronize();
    return cudaPeekAtLastError();
}

#ifdef __cplusplus
}
#endif
