#pragma once
#include <SDL3/SDL.h>
#include <cstdint>

class Audio {
public:
    Audio();
    ~Audio();

    bool init();
    void playBeep();
    void stopBeep();
    void update();
    bool isPlaying() const { return is_playing; }

private:
    SDL_AudioDeviceID device_id = 0;
    SDL_AudioStream* stream = nullptr;

    bool is_playing = false;
    float phase = 0.0f;
    float frequency = 440.0f;
    float sample_rate = 44100.0f;
};