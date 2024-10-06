/*
 * Sound.h
 *
 * Created by miles
*/

#pragma once

#include <array>

#include <SDL_audio.h>

#include "minivorbis.h"
#include "Asset.h"


class Sound {
    Sound(const Sound&) = delete;
    Sound(const Sound&&) = delete;
    void operator =(const Sound&) = delete;
public:
    static constexpr uint32_t MAX_SFX = 8;
    struct SFX {
        Sound* sound;
        uint32_t loop;
        uint64_t audioPos;
        float volume;
        Vector3 soundPos;
        uint64_t fadeInPos;
        uint64_t fadeInTime;
    };

    Sound(const char *name);

    static bool Init();
    static uint32_t Loop(Sound* sound, float fadeInTime = 0);
    static uint32_t Play(Sound* sound, Vector3 pos, float volume = 1.0f);
    static bool Stop(uint32_t handle);

    static void Pause() { SDL_PauseAudioDevice(audioDeviceId, true); }
    static void Resume() { SDL_PauseAudioDevice(audioDeviceId, false); }

    static void Volume(uint32_t handle, float v) { sfx[handle].volume = v; }


    static void Callback(void* userdata, Uint8* stream, int len);

private:
    static SDL_AudioDeviceID audioDeviceId;
    static ov_callbacks ovCallbacks;

    int16_t* pcm;
    uint64_t pcl;

    std::string file;
    Asset* oggFile;
    OggVorbis_File ovf;
    bool bReady;

    static std::array<SFX,MAX_SFX> sfx;
};
