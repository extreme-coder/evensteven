#pragma once

#include "audio_io.h"

namespace evensteven {

AudioBuffer to_mono(const AudioBuffer& buf);
AudioBuffer resample(const AudioBuffer& buf, int target_rate);
AudioBuffer normalize_peak(const AudioBuffer& buf, float target = 1.0f);
float estimate_noise_floor_db(const AudioBuffer& buf);

} // namespace evensteven
