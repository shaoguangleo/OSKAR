/*
 * Copyright (c) 2012-2013, The University of Oxford
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

#ifndef OSKAR_STATION_WORK_H_
#define OSKAR_STATION_WORK_H_

/**
 * @file oskar_station_work.h
 */

#include <oskar_global.h>
#include <oskar_mem.h>

#ifdef __cplusplus
extern "C" {
#endif

struct oskar_StationWork;
#ifndef OSKAR_STATION_WORK_TYPEDEF_
#define OSKAR_STATION_WORK_TYPEDEF_
typedef struct oskar_StationWork oskar_StationWork;
#endif /* OSKAR_STATION_WORK_TYPEDEF_ */

/**
 * @brief Creates a station work buffer structure.
 *
 * @details
 * This function initialises a structure to hold work buffers that are
 * used when calculating station beam data.
 *
 * @param[in,out] work      Pointer to structure to be initialised.
 * @param[in]     type      OSKAR memory type ID.
 * @param[in]     location  OSKAR memory location ID.
 * @param[in,out]  status   Status return code.
 */
OSKAR_EXPORT
oskar_StationWork* oskar_station_work_create(int type, int location,
        int* status);

/**
 * @brief Frees memory in a station work buffer structure.
 *
 * @details
 * This function frees memory in a data structure used to hold work buffers
 * for calculating a station beam.
 *
 * @param[in,out]  work   Pointer to structure containing memory to free.
 * @param[in,out]  status   Status return code.
 */
OSKAR_EXPORT
void oskar_station_work_free(oskar_StationWork* work, int* status);


/* Accessors. */

OSKAR_EXPORT
oskar_Mem* oskar_station_work_horizon_mask(oskar_StationWork* work);

OSKAR_EXPORT
const oskar_Mem* oskar_station_work_horizon_mask_const(
        const oskar_StationWork* work);

OSKAR_EXPORT
oskar_Mem* oskar_station_work_source_horizontal_x(oskar_StationWork* work);

OSKAR_EXPORT
const oskar_Mem* oskar_station_work_source_horizontal_x_const(
        const oskar_StationWork* work);

OSKAR_EXPORT
oskar_Mem* oskar_station_work_source_horizontal_y(oskar_StationWork* work);

OSKAR_EXPORT
const oskar_Mem* oskar_station_work_source_horizontal_y_const(
        const oskar_StationWork* work);

OSKAR_EXPORT
oskar_Mem* oskar_station_work_source_horizontal_z(oskar_StationWork* work);

OSKAR_EXPORT
const oskar_Mem* oskar_station_work_source_horizontal_z_const(
        const oskar_StationWork* work);

#ifdef __cplusplus
}
#endif

#endif /* OSKAR_STATION_WORK_H_ */
