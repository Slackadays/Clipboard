/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
    Copyright (C) 2023 Jackson Huff and other contributors on GitHub.com
    SPDX-License-Identifier: GPL-3.0-or-later
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/
#include "../clipboard.hpp"
#include <alsa/asoundlib.h>

void dummy_handler(const char* file, int line, const char* function, int err, const char* fmt, ...) {}

bool playAsyncSoundEffect(const std::valarray<short>& samples) {
#if defined(USE_ALSA)
    if (fork()) return true;

    snd_lib_error_set_handler(dummy_handler); // suppress errors in console

    snd_pcm_t* device;
    if (snd_pcm_open(&device, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) std::_Exit(EXIT_FAILURE);
    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(device, params);
    snd_pcm_hw_params_set_access(device, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(device, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(device, params, 2);
    snd_pcm_hw_params_set_rate(device, params, 44100, 0);
    snd_pcm_hw_params_set_periods(device, params, 16, 0);
    snd_pcm_hw_params_set_period_time(device, params, 1024, 0);
    snd_pcm_hw_params(device, params);

    snd_pcm_writei(device, &(samples[0]), samples.size() / 2);

    snd_pcm_drain(device);

    snd_pcm_close(device);

    std::_Exit(EXIT_SUCCESS);
#else
    return false;
#endif
}