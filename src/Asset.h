/*
 * Asset.h
 *
 * Created by miles
*/

#pragma once

#include <list>


class Asset {
    Asset(const Asset&) = delete;
    Asset(const Asset&&) = delete;
    void operator =(const Asset&) = delete;
public:
    enum STATE {
        DOWNLOADING,
        COMPLETE,
        FAILED
    };
    struct _FILE {;
        uint64_t ptr;
        uint64_t size;
        void* buf;
    };

    Asset(const char* file);

    static Asset* Fetch(const char* file) {
        return &xAssets.emplace_back(file);
    }

    uint64_t GetSize() const { return uBytesTotal; }
    _FILE* GetFILE() { return &_f; }
    void* BlockingGet() { while(eState != COMPLETE) SLEEP(0); return pBuffer; }
    void OnComplete(std::function<void()> fn) { cbOnComplete.push_back(fn); }
    static uint32_t Update() { uint32_t l = 0; for(auto& a : xAssets){ a._Update(); if(a.eState == DOWNLOADING) l++; } return l; }



private:
    void _Update() {
        if(eState == COMPLETE){
            for(auto fn: cbOnComplete) fn();
            cbOnComplete.clear();
        }
    }



    _FILE _f;


    STATE eState;
    uint64_t uBytesTotal, uBytesDownloaded;
    void* pBuffer;

#ifdef __EMSCRIPTEN__
    static void OnProgress(emscripten_fetch_t *fetch);
    static void OnSuccess(emscripten_fetch_t *fetch);
    static void OnError(emscripten_fetch_t *fetch);
#endif
    std::vector<std::function<void()>> cbOnComplete;

    SDL_mutex* assetMutex;
    static std::list<Asset> xAssets;
};
