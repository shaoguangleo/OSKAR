/*
 * Copyright (c) 2016-2019, The University of Oxford
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

#include "imager/private_imager.h"
#include "imager/oskar_imager.h"

#include "imager/oskar_grid_correction.h"
#include "imager/oskar_grid_functions_pillbox.h"
#include "imager/oskar_grid_functions_spheroidal.h"
#include "math/oskar_fft.h"
#include "math/oskar_fftphase.h"
#include "mem/oskar_mem.h"
#include "utility/oskar_device.h"
#include "utility/oskar_timer.h"

#include <fitsio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static void write_plane(oskar_Imager* h, oskar_Mem* plane,
        int c, int p, int* status);


void oskar_imager_finalise(oskar_Imager* h,
        int num_output_images, oskar_Mem** output_images,
        int num_output_grids, oskar_Mem** output_grids, int* status)
{
    int c, p, i;
    size_t j, log_size = 0, length = 0;
    char* log_data;
    if (*status || !h->planes) return;

    /* Adjust normalisation if required. */
    if (h->scale_norm_with_num_input_files)
    {
        for (i = 0; i < h->num_planes; ++i)
            h->plane_norm[i] /= h->num_files;
    }

    /* Copy grids to output grid planes if given. */
    for (i = 0; (i < h->num_planes) && (i < num_output_grids); ++i)
    {
        oskar_Mem *plane = h->planes[i];
        if (!(output_grids[i]))
            output_grids[i] = oskar_mem_create(oskar_mem_type(plane),
                    OSKAR_CPU, 0, status);
        oskar_mem_copy(output_grids[i], plane, status);
        oskar_mem_scale_real(output_grids[i], 1.0 / h->plane_norm[i],
                0, oskar_mem_length(output_grids[i]), status);
    }

    /* Check if images are required. */
    if (h->fits_file[0] || output_images)
    {
        const size_t num_pix = (size_t)h->image_size * (size_t)h->image_size;
        const int plane_size = oskar_imager_plane_size(h);

        /* Finalise all the planes. */
        for (i = 0; i < h->num_planes; ++i)
        {
            oskar_Mem *plane = h->planes[i];
            oskar_imager_finalise_plane(h, plane, h->plane_norm[i], status);
            if (plane != h->planes[i])
                oskar_mem_copy(h->planes[i], plane, status);
            oskar_imager_trim_image(h, h->planes[i],
                    plane_size, h->image_size, status);
        }

        /* Copy images to output image planes if given. */
        for (i = 0; (i < h->num_planes) && (i < num_output_images); ++i)
        {
            if (!(output_images[i]))
                output_images[i] = oskar_mem_create(h->imager_prec,
                        OSKAR_CPU, num_pix, status);
            oskar_mem_ensure(output_images[i], num_pix, status);
            memcpy(oskar_mem_void(output_images[i]),
                    oskar_mem_void_const(h->planes[i]),
                    num_pix * oskar_mem_element_size(h->imager_prec));
        }

        /* Write to files if required. */
        oskar_timer_resume(h->tmr_write);
        for (c = 0, i = 0; c < h->num_im_channels; ++c)
            for (p = 0; p < h->num_im_pols; ++p, ++i)
                write_plane(h, h->planes[i], c, p, status);
        oskar_timer_pause(h->tmr_write);
    }

    /* Record time taken. */
    oskar_log_set_value_width(25);
    oskar_log_section('M', "Imager timing");
    oskar_log_value('M', 0, "Initialise", "%.3f s",
            oskar_timer_elapsed(h->tmr_init));
    oskar_log_value('M', 0, "Grid update", "%.3f s",
            oskar_timer_elapsed(h->tmr_grid_update));
    oskar_log_value('M', 0, "Grid finalise", "%.3f s",
            oskar_timer_elapsed(h->tmr_grid_finalise));
    oskar_log_value('M', 0, "Read visibility data", "%.3f s",
            oskar_timer_elapsed(h->tmr_read));
    oskar_log_value('M', 0, "Write image data", "%.3f s",
            oskar_timer_elapsed(h->tmr_write));
    oskar_log_section('M', "Imaging complete");
    if (h->output_root)
    {
        oskar_log_message('M', 0, "Output(s):");
        for (i = 0; i < h->num_im_pols; ++i)
            oskar_log_value('M', 1, "FITS file", "%s",
                    h->output_name[i]);
    }

    /* Write log to the output FITS files as HISTORY entries.
     * Replace newlines with zeros. */
    log_data = oskar_log_file_data(&log_size);
    for (j = 0; j < log_size; ++j)
    {
        if (log_data[j] == '\n') log_data[j] = 0;
        if (log_data[j] == '\r') log_data[j] = ' ';
    }
    for (i = 0; i < h->num_im_pols; ++i)
    {
        const char* line = log_data;
        length = log_size;
        for (; log_size > 0;)
        {
            const char* eol;
            fits_write_history(h->fits_file[i], line, status);
            eol = (const char*) memchr(line, '\0', length);
            if (!eol) break;
            eol += 1;
            length -= (eol - line);
            line = eol;
        }
    }
    free(log_data);

    /* Reset imager memory. */
    oskar_imager_reset_cache(h, status);
}


void oskar_imager_finalise_plane(oskar_Imager* h,
        oskar_Mem* plane, double plane_norm, int* status)
{
    if (*status) return;

    /* Apply normalisation. */
    if (plane_norm > 0.0 || plane_norm < 0.0)
    {
        oskar_timer_resume(h->tmr_grid_finalise);
        oskar_mem_scale_real(plane, 1.0 / plane_norm,
                0, oskar_mem_length(plane), status);
        oskar_timer_pause(h->tmr_grid_finalise);
    }

    /* If algorithm if DFT, we've finished here. */
    if (h->algorithm == OSKAR_ALGORITHM_DFT_2D ||
            h->algorithm == OSKAR_ALGORITHM_DFT_3D)
        return;

    /* Check plane is complex type, as plane must be gridded visibilities. */
    if (!oskar_mem_is_complex(plane))
    {
        *status = OSKAR_ERR_BAD_DATA_TYPE;
        return;
    }

    /* Check plane size is as expected. */
    const int size = oskar_imager_plane_size(h);
    if (oskar_mem_length(plane) != ((size_t)size * (size_t)size))
    {
        *status = OSKAR_ERR_DIMENSION_MISMATCH;
        return;
    }

    /* Perform FFT shift of the input grid. */
    oskar_timer_resume(h->tmr_grid_finalise);
    oskar_fftphase(size, size, plane, status);

    /* Call FFT. */
    const int fft_loc = (h->fft_on_gpu && h->num_gpus > 0) ?
            OSKAR_GPU : OSKAR_CPU;
    if (fft_loc != OSKAR_CPU)
        oskar_device_set(h->dev_loc, h->gpu_ids[0], status);
    if (!h->fft)
        h->fft = oskar_fft_create(h->imager_prec, fft_loc, 2, size, 0, status);
    oskar_fft_exec(h->fft, plane, status);

    /* Generate grid correction function if required. */
    if (!h->corr_func)
    {
        oskar_Mem* corr_func = 0;
        corr_func = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU, size, status);
        if (h->algorithm != OSKAR_ALGORITHM_FFT)
            oskar_grid_correction_function_spheroidal(size, h->oversample,
                    oskar_mem_double(corr_func, status));
        else
        {
            if (h->kernel_type == 'S')
                oskar_grid_correction_function_spheroidal(size, 0,
                        oskar_mem_double(corr_func, status));
            else if (h->kernel_type == 'P')
                oskar_grid_correction_function_pillbox(size,
                        oskar_mem_double(corr_func, status));
        }
        h->corr_func = oskar_mem_convert_precision(corr_func,
                h->imager_prec, status);
    }

    /* FFT shift again, and apply grid correction. */
    oskar_fftphase(size, size, plane, status);
    oskar_grid_correction(size, h->corr_func, plane, status);
    oskar_timer_pause(h->tmr_grid_finalise);
}


void oskar_imager_trim_image(oskar_Imager* h, oskar_Mem* plane,
        int plane_size, int image_size, int* status)
{
    if (*status) return;

    /* Get the real part only, if the plane is complex. */
    oskar_timer_resume(h->tmr_grid_finalise);
    if (oskar_mem_is_complex(plane))
    {
        size_t i;
        const size_t num_cells = (size_t)plane_size * (size_t)plane_size;
        if (oskar_mem_precision(plane) == OSKAR_DOUBLE)
        {
            double *t = oskar_mem_double(plane, status);
            for (i = 0; i < num_cells; ++i) t[i] = t[2 * i];
        }
        else
        {
            float *t = oskar_mem_float(plane, status);
            for (i = 0; i < num_cells; ++i) t[i] = t[2 * i];
        }
    }

    /* Trim to required image size. */
    const int size_diff = plane_size - image_size;
    if (size_diff > 0)
    {
        char *ptr;
        size_t in = 0, out = 0, element_size = 0;
        int i;
        ptr = oskar_mem_char(plane);
        element_size = oskar_mem_element_size(oskar_mem_precision(plane));
        in = element_size * (size_diff / 2) * (plane_size + 1);
        const size_t copy_len = element_size * image_size;
        for (i = 0; i < image_size; ++i)
        {
            /* Use memmove() instead of memcpy() to allow for overlap. */
            memmove(ptr + out, ptr + in, copy_len);
            in += plane_size * element_size;
            out += copy_len;
        }
    }
    oskar_timer_pause(h->tmr_grid_finalise);
}


void write_plane(oskar_Imager* h, oskar_Mem* plane,
        int c, int p, int* status)
{
    long firstpix[3];
    if (*status) return;
    if (!h->fits_file[p]) return;
    const int datatype = (oskar_mem_is_double(plane) ? TDOUBLE : TFLOAT);
    firstpix[0] = 1;
    firstpix[1] = 1;
    firstpix[2] = 1 + c;
    const int num_pixels = h->image_size * h->image_size;
    fits_write_pix(h->fits_file[p], datatype, firstpix, num_pixels,
            oskar_mem_void(plane), status);
}


#ifdef __cplusplus
}
#endif
