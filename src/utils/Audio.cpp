#include "utils/audio.h"
#include "utils/logger.h"
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

void Audio::update()
{
    if (!is_playing || !stream) return;

    int target_samples = 1024;
    int current_queued_bytes = SDL_GetAudioStreamQueued(stream) / sizeof(float);

    if (current_queued_bytes < target_samples) {
        int samples_to_write = (target_samples - current_queued_bytes) / sizeof(float);
        std::vector<float> buffer(samples_to_write);

        float phase_increment = frequency / sample_rate;

        for (int i = 0; i < samples_to_write; ++i) {
            // Onde carrée : amplitude 0.2 (ne pas mettre 1.0 car c'est violent pour les oreilles)
            buffer[i] = (phase < 0.5f) ? 0.2f : -0.2f;

            phase += phase_increment;
            if (phase >= 1.0f) phase -= 1.0f;
        }

        // ON ENVOIE LES DONNÉES AU STREAM
        SDL_PutAudioStreamData(stream, buffer.data(), buffer.size() * sizeof(float));
    }
}
