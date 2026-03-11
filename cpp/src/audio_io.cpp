#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#include "evensteven/audio_io.h"
#include "evensteven/logging.h"
#include <algorithm>
#include <stdexcept>
#include <filesystem>

namespace evensteven {

int AudioBuffer::num_frames() const {
    if (channels == 0) return 0;
    return static_cast<int>(samples.size()) / channels;
}

float AudioBuffer::duration_s() const {
    if (sample_rate == 0) return 0.0f;
    return static_cast<float>(num_frames()) / static_cast<float>(sample_rate);
}

static std::string get_extension(const std::string& filepath) {
    auto ext = std::filesystem::path(filepath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

bool is_supported_format(const std::string& filepath) {
    auto ext = get_extension(filepath);
    return ext == ".wav" || ext == ".flac" || ext == ".mp3";
}

AudioBuffer load_audio(const std::string& filepath) {
    auto ext = get_extension(filepath);
    AudioBuffer buf;

    if (ext == ".wav") {
        drwav wav;
        if (!drwav_init_file(&wav, filepath.c_str(), nullptr)) {
            throw std::runtime_error("Failed to open WAV file: " + filepath);
        }
        buf.sample_rate = static_cast<int>(wav.sampleRate);
        buf.channels = static_cast<int>(wav.channels);
        size_t total = static_cast<size_t>(wav.totalPCMFrameCount) * wav.channels;
        buf.samples.resize(total);
        drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, buf.samples.data());
        drwav_uninit(&wav);
    } else if (ext == ".flac") {
        unsigned int channels, sampleRate;
        drflac_uint64 totalFrames;
        float* data = drflac_open_file_and_read_pcm_frames_f32(
            filepath.c_str(), &channels, &sampleRate, &totalFrames, nullptr);
        if (!data) {
            throw std::runtime_error("Failed to open FLAC file: " + filepath);
        }
        buf.sample_rate = static_cast<int>(sampleRate);
        buf.channels = static_cast<int>(channels);
        size_t total = static_cast<size_t>(totalFrames) * channels;
        buf.samples.assign(data, data + total);
        drflac_free(data, nullptr);
    } else if (ext == ".mp3") {
        drmp3_config mp3cfg;
        drmp3_uint64 totalFrames;
        float* data = drmp3_open_file_and_read_pcm_frames_f32(
            filepath.c_str(), &mp3cfg, &totalFrames, nullptr);
        if (!data) {
            throw std::runtime_error("Failed to open MP3 file: " + filepath);
        }
        buf.sample_rate = static_cast<int>(mp3cfg.sampleRate);
        buf.channels = static_cast<int>(mp3cfg.channels);
        size_t total = static_cast<size_t>(totalFrames) * mp3cfg.channels;
        buf.samples.assign(data, data + total);
        drmp3_free(data, nullptr);
    } else {
        throw std::runtime_error("Unsupported audio format: " + ext);
    }

    log(LogLevel::INFO, "Loaded " + filepath + " (" +
        std::to_string(buf.duration_s()) + "s, " +
        std::to_string(buf.sample_rate) + " Hz, " +
        std::to_string(buf.channels) + " ch)");

    return buf;
}

AudioMetadata probe_audio(const std::string& filepath) {
    auto ext = get_extension(filepath);
    AudioMetadata meta;
    meta.filename = std::filesystem::path(filepath).filename().string();
    meta.format = ext.substr(1);

    if (ext == ".wav") {
        drwav wav;
        if (!drwav_init_file(&wav, filepath.c_str(), nullptr)) {
            throw std::runtime_error("Failed to probe WAV file: " + filepath);
        }
        meta.sample_rate = static_cast<int>(wav.sampleRate);
        meta.channels = static_cast<int>(wav.channels);
        meta.duration_s = static_cast<float>(wav.totalPCMFrameCount) / static_cast<float>(wav.sampleRate);
        drwav_uninit(&wav);
    } else if (ext == ".flac") {
        drflac* flac = drflac_open_file(filepath.c_str(), nullptr);
        if (!flac) {
            throw std::runtime_error("Failed to probe FLAC file: " + filepath);
        }
        meta.sample_rate = static_cast<int>(flac->sampleRate);
        meta.channels = static_cast<int>(flac->channels);
        meta.duration_s = static_cast<float>(flac->totalPCMFrameCount) / static_cast<float>(flac->sampleRate);
        drflac_close(flac);
    } else if (ext == ".mp3") {
        drmp3 mp3;
        if (!drmp3_init_file(&mp3, filepath.c_str(), nullptr)) {
            throw std::runtime_error("Failed to probe MP3 file: " + filepath);
        }
        meta.sample_rate = static_cast<int>(mp3.sampleRate);
        meta.channels = static_cast<int>(mp3.channels);
        drmp3_uint64 totalFrames = drmp3_get_pcm_frame_count(&mp3);
        meta.duration_s = static_cast<float>(totalFrames) / static_cast<float>(mp3.sampleRate);
        drmp3_uninit(&mp3);
    }

    return meta;
}

} // namespace evensteven
