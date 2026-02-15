#include "utils/audio.h"
#include "utils/Logger.h"
#include <cmath>
#include <vector>

Audio::Audio() : device_id(0), stream(nullptr){}

Audio::~Audio() {
    if (stream) {
        SDL_DestroyAudioStream(stream);
    }
}

bool Audio::init() {
    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq = 44100;

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, this);

    if (!stream) {
        LOG_ERROR("Failed to open audio: {}", SDL_GetError());
        return false;
    }
    device_id = SDL_GetAudioStreamDevice(stream);
    sample_rate = static_cast<float>(spec.freq);

    SDL_ResumeAudioStreamDevice(stream);

    LOG_INFO("Audio initialized: {} Hz, Mono (SDL3 Stream)", spec.freq);
    return true;
}

void Audio::playBeep() {
    if (!is_playing) {
        is_playing = true;
        phase = 0.0f;
        LOG_DEBUG("Beep started");
    }
}

void Audio::stopBeep() {
    if (is_playing) {
        is_playing = false;
        LOG_DEBUG("Beep stopped");
    }
}

void Audio::update() {
    if (!is_playing || !stream) return;

    int target_samples = 1024;
    int queued_samples = SDL_GetAudioStreamQueued(stream) / sizeof(float);

    if (queued_samples < target_samples) {
        int samples_to_write = target_samples - queued_samples;
        std::vector<float> buffer(samples_to_write);

        float phase_increment = frequency / sample_rate;

        for (int i = 0; i < samples_to_write; ++i) {
            float raw_square = (phase < 0.5f) ? 1.0f : -1.0f;

            float smoothed = raw_square;
            if (phase < 0.02f || (phase > 0.48f && phase < 0.52f) || phase > 0.98f) {
                float t = std::fmod(phase, 0.02f) / 0.02f;
                smoothed = raw_square * t;
            }

            buffer[i] = smoothed * 0.15f;  // Amplitude réduite (15% au lieu de 20%)

            phase += phase_increment;
            if (phase >= 1.0f) phase -= 1.0f;
        }

        SDL_PutAudioStreamData(stream, buffer.data(), buffer.size() * sizeof(float));
    }
}
