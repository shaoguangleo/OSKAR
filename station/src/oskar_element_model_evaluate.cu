/*
 * Copyright (c) 2012, The University of Oxford
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

#include "station/oskar_element_model_evaluate.h"
#include "station/oskar_evaluate_spline_pattern.h"

#define PIf 3.14159265358979323846f
#define PI  3.14159265358979323846

__global__
void oskar_cudak_hor_lmn_to_modified_theta_phi_f(const int num,
        const float* l, const float* m, const float* n,
        const float delta_phi, float* theta, float* phi)
{
    // Get the position ID that this thread is working on.
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i >= num) return;

    // Get the data.
    float x = l[i];
    float y = m[i];
    float z = n[i];

    // Cartesian to spherical (with orientation offset).
    float p = atan2f(y, x) + delta_phi;
    p = fmodf(p, 2.0f * PIf);
    if (p < 0) p += 2.0f * PIf; // Get phi in range 0 to 2 pi.
    x = sqrtf(x*x + y*y);
    y = atan2f(x, z); // Theta.
    phi[i] = p;
    theta[i] = y;
}

__global__
void oskar_cudak_hor_lmn_to_modified_theta_phi_d(const int num,
        const double* l, const double* m, const double* n,
        const double delta_phi, double* theta, double* phi)
{
    // Get the position ID that this thread is working on.
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i >= num) return;

    // Get the data.
    double x = l[i];
    double y = m[i];
    double z = n[i];

    // Cartesian to spherical (with orientation offset).
    double p = atan2(y, x) + delta_phi;
    p = fmod(p, 2.0 * PI);
    if (p < 0) p += 2.0 * PI; // Get phi in range 0 to 2 pi.
    x = sqrt(x*x + y*y);
    y = atan2(x, z); // Theta.
    phi[i] = p;
    theta[i] = y;
}

#ifdef __cplusplus
extern "C" {
#endif

int oskar_hor_lmn_to_modified_theta_phi(oskar_Mem* theta, oskar_Mem* phi,
        double delta_phi, const oskar_Mem* l, const oskar_Mem* m,
        const oskar_Mem* n)
{
    int error, num_sources, num_blocks, num_threads, type;

    /* Sanity check on inputs. */
    if (!theta || !phi || !l || !m || !n)
        return OSKAR_ERR_INVALID_ARGUMENT;

    /* Get data type and number of sources. */
    type = l->type;
    num_sources = l->num_elements;

    if (type == OSKAR_SINGLE)
    {
        num_threads = 256;
        num_blocks = (num_sources + num_threads - 1) / num_threads;
        oskar_cudak_hor_lmn_to_modified_theta_phi_f
        OSKAR_CUDAK_CONF(num_blocks, num_threads) (num_sources,
                ((const float*)l->data), ((const float*)m->data),
                ((const float*)n->data), (float)delta_phi,
                ((float*)theta->data), ((float*)phi->data));
    }
    else if (type == OSKAR_DOUBLE)
    {
        num_threads = 256;
        num_blocks = (num_sources + num_threads - 1) / num_threads;
        oskar_cudak_hor_lmn_to_modified_theta_phi_d
        OSKAR_CUDAK_CONF(num_blocks, num_threads) (num_sources,
                ((const double*)l->data), ((const double*)m->data),
                ((const double*)n->data), delta_phi,
                ((double*)theta->data), ((double*)phi->data));
    }
    else
        return OSKAR_ERR_BAD_DATA_TYPE;

    cudaDeviceSynchronize();
    error = (int) cudaPeekAtLastError();
    return error;
}

// Device function to evaluate the response of the X-dipole (single precision).
__device__
void oskar_cudaf_evaluate_dipole_pattern_x_f(const float theta,
        const float phi, float2* response_theta, float2* response_phi)
{

}

// Device function to evaluate the response of the Y-dipole (single precision).
__device__
void oskar_cudaf_evaluate_dipole_pattern_y_f(const float theta,
        const float phi, float2* response_theta, float2* response_phi)
{

}


__global__
void oskar_cudak_evaluate_dipole_pattern_f(const int num, const int port,
        const float* theta, const float* phi, float2* response_theta,
        float2* response_phi)
{
    // Get the position ID that this thread is working on.
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i >= num) return;
}

int oskar_element_model_evaluate(const oskar_ElementModel* model, oskar_Mem* G,
        double orientation_x, double orientation_y, const oskar_Mem* l,
        const oskar_Mem* m, const oskar_Mem* n, oskar_Mem* theta,
        oskar_Mem* phi)
{
    int error, num_blocks, num_threads;

    /* Check that the output array is complex. */
    if (!oskar_mem_is_complex(G->type))
        return OSKAR_ERR_BAD_DATA_TYPE;

    /* Check that all arrays are on the GPU. */
    if (l->location != OSKAR_LOCATION_GPU ||
            m->location != OSKAR_LOCATION_GPU ||
            n->location != OSKAR_LOCATION_GPU ||
            G->location != OSKAR_LOCATION_GPU)
        return OSKAR_ERR_BAD_LOCATION;

    /* Convert dipole axis orientations to delta values in phi. */
    orientation_x -= 0.5 * PI;

    /* Evaluate polarised response if output array is matrix type. */
    if (oskar_mem_is_matrix(G->type) && model->polarised)
    {
        /* Compute modified theta and phi coordinates for dipole X. */
        error = oskar_hor_lmn_to_modified_theta_phi(theta, phi,
                orientation_x, l, m, n);
        if (error) return error;

        /* Check if spline data present for dipole X. */
        if (model->theta_re_x.coeff.data && model->theta_im_x.coeff.data &&
                model->phi_re_x.coeff.data && model->phi_im_x.coeff.data)
        {
            /* Evaluate spline pattern for dipole X. */
            error = oskar_spline_data_evaluate(G, 0, 8, &model->theta_re_x,
                    theta, phi);
            if (error) return error;
            error = oskar_spline_data_evaluate(G, 1, 8, &model->theta_im_x,
                    theta, phi);
            if (error) return error;
            error = oskar_spline_data_evaluate(G, 2, 8, &model->phi_re_x,
                    theta, phi);
            if (error) return error;
            error = oskar_spline_data_evaluate(G, 3, 8, &model->phi_im_x,
                    theta, phi);
            if (error) return error;
        }
        else
        {
            /* Evaluate tapered dipole pattern for dipole X. */
        }

        /* Compute modified theta and phi coordinates for dipole Y. */
        error = oskar_hor_lmn_to_modified_theta_phi(theta, phi,
                orientation_y, l, m, n);
        if (error) return error;

        /* Check if spline data present for dipole Y. */
        if (model->theta_re_y.coeff.data && model->theta_im_y.coeff.data &&
                model->phi_re_y.coeff.data && model->phi_im_y.coeff.data)
        {
            /* Evaluate spline pattern for dipole Y. */
            error = oskar_spline_data_evaluate(G, 4, 8, &model->theta_re_y,
                    theta, phi);
            if (error) return error;
            error = oskar_spline_data_evaluate(G, 5, 8, &model->theta_im_y,
                    theta, phi);
            if (error) return error;
            error = oskar_spline_data_evaluate(G, 6, 8, &model->phi_re_y,
                    theta, phi);
            if (error) return error;
            error = oskar_spline_data_evaluate(G, 7, 8, &model->phi_im_y,
                    theta, phi);
            if (error) return error;
        }
        else
        {
            /* Evaluate tapered dipole pattern for dipole Y. */
        }
    }

    /* Evaluate scalar response if output array is scalar type. */
    else if (oskar_mem_is_scalar(G->type) && !model->polarised)
    {
        /* Not yet implemented. */
        return OSKAR_ERR_FUNCTION_NOT_AVAILABLE;
    }
    else
        return OSKAR_ERR_BAD_DATA_TYPE;

    return 0;
}

#ifdef __cplusplus
}
#endif