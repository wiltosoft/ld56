/*
 * Sound.cpp
 *
 * Created by miles
*/

#include "Sound.h"


SDL_AudioDeviceID Sound::audioDeviceId = 0;
std::array<Sound::SFX,Sound::MAX_SFX> Sound::sfx;

extern "C" {
    size_t __mv_fread(void *f, size_t sz, size_t ct, void *buf);
    int __mv_fseek(void *f, int64_t off, int sk);
    int __mv_fclose(void *f);
    long __mv_ftell(void *f);
}

ov_callbacks Sound::ovCallbacks = {__mv_fread, __mv_fseek, __mv_fclose, __mv_ftell};


Sound::Sound(const char *name)
{
    bReady = false;
    file = file + "assets/sounds/" + name + ".ogg";
    oggFile = Asset::Fetch(file.c_str());

    oggFile->OnComplete([this](){
        ov_open_callbacks(oggFile->GetFILE(), &ovf, nullptr, oggFile->GetSize(), ovCallbacks);

        vorbis_info* info = ov_info(&ovf, -1);
        pcl = ov_pcm_total(&ovf, 0);
        int l = pcl * info->channels * 2;

        _INFO("Sound %s: %d Hz, %d channels, %d kbit/s", file.c_str(), info->rate, info->channels, info->bitrate_nominal / 1024);
        if(info->channels != 1) _ERROR("Only support MONO audio");
        if(info->rate != 48000) _ERROR("Only support 48kHz audio");

        int section = 0;
        auto bp = pcm = (int16_t*)malloc(l);
        while(long bytes = ov_read(&ovf, (char*)bp, l, 0, 2, 1, &section)){
            bp += bytes / 2;
            l -= bytes / 2;
        }
        ov_clear(&ovf);

        bReady = true;
    });
}

bool Sound::Init()
{
    SDL_AudioSpec desired = {
        48000, AUDIO_S16, 4, 0, 4800, 0, 0, Sound::Callback, nullptr
    };
    SDL_AudioSpec obtained;
    audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);

    if(audioDeviceId == 0){
        _ERROR("Failed to connect to sound device");
        return false;
    }

    SDL_PauseAudioDevice(audioDeviceId, false);

    return true;
}


void Sound::Callback(void *userdata, Uint8 *stream, int len)
{
    static uint64_t s = 0;

    auto sd = (int16_t*)stream;
    len /= 2;
    for (auto o = 0; o < len; o += 4) {
        int16_t fl = 0, fr = 0, bl = 0, br = 0;
        for(auto& ss : sfx) {
            if(ss.sound && ss.sound->bReady) {
                auto v = ss.volume * pow(1+length(ss.soundPos), -2);
                if(ss.fadeInPos < ss.fadeInTime){
                    v *= (float)ss.fadeInPos / ss.fadeInTime;
                    ss.fadeInPos++;
                }
                const auto s = v * ss.sound->pcm[ss.audioPos];
                fl += s * std::max(0.0f, (float)dot(normalize(ss.soundPos), normalize(Vector3{-1, 0, 1})));
                fr += s * std::max(0.0f, (float)dot(normalize(ss.soundPos), normalize(Vector3{1, 0, 1})));
                bl += s * std::max(0.0f, (float)dot(normalize(ss.soundPos), normalize(Vector3{-1, 0, -1})));
                br += s * std::max(0.0f, (float)dot(normalize(ss.soundPos), normalize(Vector3{1, 0, -1})));

                ++ss.audioPos;
                if(ss.audioPos > ss.sound->pcl){
                    ss.audioPos = 0;
                    if(--ss.loop == 0){
                        ss.sound = nullptr;
                        break;
                    }
                }
            }
        }
        sd[o] = fl;
        sd[o + 1] = fr;
        sd[o + 2] = bl;
        sd[o + 3] = br;
//        ss->pcp += 2;
//        if(ss->pcp >= ss->pcl * 2) ss->pcp = 0;
    }
}


uint32_t Sound::Loop(Sound *sound, float fadeInTime)
{
    uint32_t i = 0;
    for(; i < MAX_SFX; i++)
        if(sfx[i].sound == nullptr)
            break;
    if(i >= MAX_SFX)
        return UINT32_MAX;

    sfx[i].volume = 1;
    sfx[i].audioPos = 0;
    sfx[i].loop = UINT32_MAX;
    sfx[i].soundPos = {0, 0, 0.5};
    sfx[i].sound = sound;
    sfx[i].fadeInPos = 0;
    sfx[i].fadeInTime = fadeInTime * 48000;

    return i;
}


uint32_t Sound::Play(Sound *sound, Vector3 pos, float volume)
{
    uint32_t i = 0;
    for(; i < MAX_SFX; i++)
        if(sfx[i].sound == nullptr)
            break;
    if(i >= MAX_SFX)
        return UINT32_MAX;

    sfx[i].volume = volume;
    sfx[i].audioPos = 0;
    sfx[i].loop = 1;
    sfx[i].soundPos = pos;
    sfx[i].sound = sound;
    sfx[i].fadeInPos = 0;
    sfx[i].fadeInTime = 0;

    return i;
}


bool Sound::Stop(uint32_t handle)
{
    if(handle >= MAX_SFX)
        return false;

    if(sfx[handle].sound == nullptr)
        return false;

    sfx[handle].sound = nullptr;
    return true;
}
