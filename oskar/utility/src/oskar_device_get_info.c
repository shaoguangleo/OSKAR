/*
 * Copyright (c) 2018-2019, The University of Oxford
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "utility/private_device.h"
#include "utility/oskar_device.h"

#ifdef OSKAR_HAVE_CUDA
#include <cuda_runtime_api.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static unsigned int oskar_get_num_cuda_cores(int major, int minor);

void oskar_device_get_info_cl(oskar_Device* device)
{
#ifdef OSKAR_HAVE_OPENCL
#define GDI clGetDeviceInfo
    char vd[2];
    char* t = 0;
    size_t i, len = 0, st = 0;
    cl_uint ui = 0;
    cl_ulong ul = 0;
    cl_device_type d_type;
    cl_device_id id = device->device_id_cl;
    device->platform_type = 'O';
    GDI(id, CL_DEVICE_VENDOR, 0, 0, &len);
    device->vendor = (char*) realloc(device->vendor, len);
    GDI(id, CL_DEVICE_VENDOR, len, device->vendor, 0);
    for (i = 0; i < 2; ++i) vd[i] = toupper(device->vendor[i]);
    device->is_nv = !strncmp(vd, "NV", 2);
    GDI(id, CL_DEVICE_NAME, 0, 0, &len);
    device->name = (char*) realloc(device->name, len);
    GDI(id, CL_DEVICE_NAME, len, device->name, 0);
    GDI(id, CL_DEVICE_VERSION, 0, 0, &len);
    device->cl_version = (char*) realloc(device->cl_version, len);
    GDI(id, CL_DEVICE_VERSION, len, device->cl_version, 0);
    GDI(id, CL_DRIVER_VERSION, 0, 0, &len);
    device->cl_driver_version = (char*) realloc(device->cl_driver_version, len);
    GDI(id, CL_DRIVER_VERSION, len, device->cl_driver_version, 0);
    GDI(id, CL_DEVICE_EXTENSIONS, 0, 0, &len); t = (char*) realloc(t, len);
    GDI(id, CL_DEVICE_EXTENSIONS, len, t, 0);
    device->supports_double = strstr(t, "cl_khr_fp64") ? 1 : 0;
    device->supports_atomic32 =
            strstr(t, "cl_khr_global_int32_base_atomics") ? 1 : 0;
    device->supports_atomic64 =
            strstr(t, "cl_khr_int64_base_atomics") ? 1 : 0;
    GDI(id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &ul, 0);
    device->global_mem_size = (size_t) ul;
    GDI(id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &ul, 0);
    device->global_mem_cache_size = (size_t) ul;
    GDI(id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ul, 0);
    device->local_mem_size = (size_t) ul;
    GDI(id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ul, 0);
    device->max_mem_alloc_size = (size_t) ul;
    GDI(id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &ui, 0);
    device->max_clock_freq_kHz = (int) ui * 1000;
    GDI(id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &ui, 0);
    device->max_compute_units = (int) ui;
    GDI(id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &st, 0);
    device->max_work_group_size = st;
    GDI(id, CL_DEVICE_TYPE, sizeof(cl_device_type), &d_type, 0);
    if ((d_type & CL_DEVICE_TYPE_GPU) == CL_DEVICE_TYPE_GPU)
        device->device_type = 'G';
    if ((d_type & CL_DEVICE_TYPE_CPU) == CL_DEVICE_TYPE_CPU)
        device->device_type = 'C';
    if ((d_type & CL_DEVICE_TYPE_ACCELERATOR) == CL_DEVICE_TYPE_ACCELERATOR)
        device->device_type = 'A';
    GDI(id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &ui, 0);
    size_t* max_dims = (size_t*) calloc(ui, sizeof(size_t));
    GDI(id, CL_DEVICE_MAX_WORK_ITEM_SIZES, ui * sizeof(size_t), max_dims, 0);
    for (i = 0; (i < ui) && (i < 3); ++i)
        device->max_local_size[i] = max_dims[i];
    free(max_dims);
    free(t);
#undef GDI
#endif
    device->init = 1;
}

void oskar_device_get_info_cuda(oskar_Device* device)
{
#ifdef OSKAR_HAVE_CUDA
    struct cudaDeviceProp prop;
    cudaDriverGetVersion(&device->cuda_driver_version);
    cudaRuntimeGetVersion(&device->cuda_runtime_version);
    cudaGetDeviceProperties(&prop, device->index);
    device->name = (char*) realloc(device->name, 1 + strlen(prop.name));
    device->vendor = (char*) realloc(device->vendor, 1 + strlen("NVIDIA"));
    strcpy(device->name, prop.name);
    strcpy(device->vendor, "NVIDIA");
    device->is_nv = 1;
    device->platform_type = 'C';
    device->device_type = 'G';
    device->compute_capability[0] = prop.major;
    device->compute_capability[1] = prop.minor;
    device->supports_double = 0;
    if (prop.major >= 2 || prop.minor >= 3)
        device->supports_double = 1;
    device->supports_atomic32 = 1;
    device->supports_atomic64 = 1;
    device->global_mem_cache_size = (size_t) prop.l2CacheSize;
    device->local_mem_size = prop.sharedMemPerBlock;
    device->max_work_group_size = (size_t) prop.maxThreadsPerBlock;
    device->max_local_size[0] = prop.maxThreadsDim[0];
    device->max_local_size[1] = prop.maxThreadsDim[1];
    device->max_local_size[2] = prop.maxThreadsDim[2];
    device->max_compute_units = prop.multiProcessorCount;
    device->max_clock_freq_kHz = prop.clockRate;
    device->memory_clock_freq_kHz = prop.memoryClockRate;
    device->memory_bus_width = prop.memoryBusWidth;
    device->num_registers = (unsigned int) prop.regsPerBlock;
    device->warp_size = prop.warpSize;
    cudaMemGetInfo(&device->global_mem_free_size, &device->global_mem_size);
#endif
    device->num_cores = device->max_compute_units * oskar_get_num_cuda_cores(
            device->compute_capability[0], device->compute_capability[1]);
    device->init = 1;
}

static unsigned int oskar_get_num_cuda_cores(int major, int minor)
{
    switch ((major << 4) + minor)
    {
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
        return 8;
    case 0x20:
        return 32;
    case 0x21:
        return 48;
    case 0x30:
    case 0x32:
    case 0x35:
    case 0x37:
        return 192;
    case 0x50:
    case 0x52:
    case 0x53:
    case 0x61:
    case 0x62:
        return 128;
    case 0x60:
    case 0x70:
    case 0x72:
    case 0x75:
        return 64;
    default:
        return 0;
    }
}

#ifdef __cplusplus
}
#endif
