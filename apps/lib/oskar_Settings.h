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

#ifndef OSKAR_SETTINGS_H_
#define OSKAR_SETTINGS_H_

#include <QtCore/QString>
#include <QtCore/QSettings>

/// Container class for image settings group.
class oskar_ImageSettings
{
    public:
        void load(const QSettings& settings);

    public:
        double fov_deg() const { return _fov_deg; }
        void set_fov_deg(const double value) { _fov_deg = value; }

        unsigned size() const { return _size; }
        void set_size(const unsigned value) { _size = value; }

        bool make_snapshots() const { return _make_snapshots; }
        void set_make_snapshots(const bool value) { _make_snapshots = value; }

        unsigned dumps_per_snapshot() const { return _dumps_per_snapshot; }
        void set_dumps_per_snapshot(const unsigned value) { _dumps_per_snapshot = value; }

        bool scan_frequencies() const { return _scan_frequencies; }
        void set_scan_frequencies(const bool value) { _scan_frequencies = value; }

        QString filename() const { return _filename; }
        void set_filename(const QString& value)  { _filename = value; }

    private:
        double   _fov_deg;
        unsigned _size;
        bool     _make_snapshots;
        unsigned _dumps_per_snapshot;
        bool     _scan_frequencies;
        QString  _filename;
};


/**
 *FIXME
 *FIXME store settings categories in separate classes / structures.
 *FIXME i.e. settings.image.fov_deg();
 *FIXME
 */
class oskar_Settings
{
    public:
        oskar_Settings(const QString& filename = QString());
        ~oskar_Settings();

    public:
        int load(const QString& filename = QString());

        int check() const;

        void print() const;

    public:
        static const double deg2rad = 0.0174532925199432957692;

        QString sky_file() const { return _sky_file; }
        void set_sky_file(const QString& value) { _sky_file = value; }

        QString telescope_file() const { return _telescope_file; }
        void set_telescope_file(const QString& value) { _telescope_file = value; }

        double longitude_deg() const { return _longitude_deg; }
        void set_longitude_deg(const double value) { _longitude_deg = value; }
        double latitude_deg() const { return _latitude_deg; }
        void set_latitude_deg(const double value) { _latitude_deg = value; }
        double longitude_rad() const { return _longitude_deg * deg2rad; }
        double latitude_rad() const { return _latitude_deg * deg2rad; }

        QString station_dir() const { return _station_dir; }
        void set_station_dir(const QString& value) { _station_dir = value; }
        bool disable_station_beam() const { return _disable_station_beam; }
        void set_disable_station_beam(const bool value)
        { _disable_station_beam = value; }


        double frequency(const unsigned channel) const
        { return _start_frequency + channel * _frequency_inc; }
        double start_frequency() const { return _start_frequency; }
        void set_start_frequency(const double value) { _start_frequency = value; }
        unsigned num_channels() const { return _num_channels; }
        void set_num_channels(const unsigned value) { _num_channels = value; }
        double frequency_inc() const { return _frequency_inc; }
        void set_frequency_inc(const double value) { _frequency_inc = value; }
        double channel_bandwidth() const { return _channel_bandwidth; }
        void set_channel_bandwidth(const double value) { _channel_bandwidth = value; }
        double ra0_deg() const { return _ra0_deg; }
        void set_ra0_deg(const double value) { _ra0_deg = value; }
        double dec0_deg() const { return _dec0_deg; }
        void set_dec0_deg(const double value) { _dec0_deg = value; }
        double ra0_rad() const { return _ra0_deg * deg2rad; }
        double dec0_rad() const { return _dec0_deg * deg2rad; }
        double obs_length_sec() const { return _obs_length_sec; }
        void set_obs_length_sec(const double value) { _obs_length_sec = value; }
        double obs_length_days() const
        { return _obs_length_sec / (24.0 * 60.0 * 60.0); }
        double obs_start_mjd_utc() const { return _obs_start_mjd_utc; }
        void set_obs_Start_mjd_utc(const double value) { _obs_start_mjd_utc = value; }

        unsigned num_vis_dumps() const { return _num_vis_dumps; }
        void set_num_vis_dumps(const unsigned value) { _num_vis_dumps = value; }
        unsigned num_vis_ave() const { return _num_vis_ave; }
        void set_num_vis_ave(const unsigned value)  { _num_vis_ave = value; }
        unsigned num_fringe_ave() const { return _num_fringe_ave; }
        void set_num_fringe_ave(const unsigned value) { _num_fringe_ave = value; }

        bool double_precision() { return _prec_double; }

        QString oskar_vis_filename() const { return _oskar_vis_filename; }
        void set_oskar_vis_filename(const QString& value)
        { _oskar_vis_filename = value; }
        QString ms_filename() const { return _ms_filename; }
        void set_ms_filename(const QString& value) { _ms_filename = value; }

        const oskar_ImageSettings& image() const { return _image; }
        oskar_ImageSettings& image() { return _image; }

    private:
        QString _filename;

        QString _sky_file;

        QString _telescope_file;
        double _longitude_deg;
        double _latitude_deg;

        QString _station_dir;
        bool _disable_station_beam;

        double _start_frequency;
        unsigned _num_channels;
        double _frequency_inc;
        double _channel_bandwidth;
        double _ra0_deg;
        double _dec0_deg;
        double _obs_length_sec;
        double _obs_start_mjd_utc;

        unsigned _num_vis_dumps;
        unsigned _num_vis_ave;
        unsigned _num_fringe_ave;

        bool _prec_double;

        QString _oskar_vis_filename;
        QString _ms_filename;

        oskar_ImageSettings _image;
};

#endif // OSKAR_SETTINGS_H_
