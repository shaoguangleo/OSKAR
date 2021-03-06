/*
 * Copyright (c) 2011-2019, The University of Oxford
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

#ifdef OSKAR_HAVE_CUDA
#include <cuda_runtime_api.h>
#endif

#include "mem/oskar_mem.h"
#include "mem/private_mem.h"
#include "utility/oskar_device.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_mem_realloc(oskar_Mem* mem, size_t num_elements, int* status)
{
    if (*status) return;

    /* Check if the structure owns the memory it points to. */
    if (mem->owner == 0)
    {
        *status = OSKAR_ERR_MEMORY_NOT_ALLOCATED;
        return;
    }

    /* Get size of new and old memory blocks. */
    const size_t element_size = oskar_mem_element_size(mem->type);
    if (element_size == 0)
    {
        *status = OSKAR_ERR_BAD_DATA_TYPE;
        return;
    }
    const size_t new_size = num_elements * element_size;
    const size_t old_size = mem->num_elements * element_size;

    /* Do nothing if new size and old size are the same. */
    if (new_size == old_size)
        return;

    /* Check memory location. */
    if (mem->location == OSKAR_CPU)
    {
        /* Reallocate the memory. */
        void* mem_new = realloc(mem->data, new_size);
        if (!mem_new && (new_size > 0))
        {
            *status = OSKAR_ERR_MEMORY_ALLOC_FAILURE;
            return;
        }

        /* Initialise the new memory if it's larger than the old block. */
        if (new_size > old_size)
            memset((char*)mem_new + old_size, 0, new_size - old_size);

        /* Set the new meta-data. */
        mem->data = (new_size > 0) ? mem_new : 0;
        mem->num_elements = num_elements;
    }
    else if (mem->location == OSKAR_GPU)
    {
#ifdef OSKAR_HAVE_CUDA
        /* Allocate and initialise a new block of memory. */
        void* mem_new = NULL;
        if (new_size > 0)
        {
            const int cuda_error = (int)cudaMalloc(&mem_new, new_size);
            if (cuda_error)
            {
                *status = cuda_error;
                return;
            }
            if (!mem_new)
            {
                *status = OSKAR_ERR_MEMORY_ALLOC_FAILURE;
                return;
            }
        }

        /* Copy contents of old block to new block. */
        const size_t copy_size = (old_size > new_size) ? new_size : old_size;
        if (copy_size > 0)
        {
            const int cuda_error = (int)cudaMemcpy(mem_new,
                    mem->data, copy_size, cudaMemcpyDeviceToDevice);
            if (cuda_error)
                *status = cuda_error;
        }

        /* Free the old block. */
        const int cuda_error = (int)cudaFree(mem->data);
        if (cuda_error && !*status) *status = cuda_error;

        /* Set the new meta-data. */
        mem->data = mem_new;
        mem->num_elements = num_elements;
#else
        *status = OSKAR_ERR_CUDA_NOT_AVAILABLE;
#endif
    }
    else if (mem->location & OSKAR_CL)
    {
#ifdef OSKAR_HAVE_OPENCL
        /* Allocate and initialise a new block of memory. */
        cl_event event;
        cl_int error = 0;
        cl_mem mem_new;
        mem_new = clCreateBuffer(oskar_device_context_cl(),
                CL_MEM_READ_WRITE, new_size, NULL, &error);
        if (error != CL_SUCCESS)
        {
            *status = OSKAR_ERR_MEMORY_ALLOC_FAILURE;
            return;
        }

        /* Copy contents of old block to new block. */
        const size_t copy_size = (old_size > new_size) ? new_size : old_size;
        if (copy_size > 0)
        {
            error = clEnqueueCopyBuffer(oskar_device_queue_cl(), mem->buffer,
                    mem_new, 0, 0, copy_size, 0, NULL, &event);
            if (error != CL_SUCCESS)
                *status = OSKAR_ERR_MEMORY_COPY_FAILURE;
        }

        /* Free the old buffer. */
        if (mem->buffer)
            clReleaseMemObject(mem->buffer);

        /* Set the new meta-data. */
        mem->buffer = mem_new;
        mem->data = (void*) (mem->buffer);
        mem->num_elements = num_elements;
#else
        *status = OSKAR_ERR_OPENCL_NOT_AVAILABLE;
#endif
    }
    else
    {
        *status = OSKAR_ERR_BAD_LOCATION;
    }
}

#ifdef __cplusplus
}
#endif
