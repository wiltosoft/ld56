/*
 * Game.h
 *
 * Created by miles
*/

#pragma once

#include "global.h"
#include "Scene.h"
#include "Player.h"
#include "GUI.h"


class Game {
public:
    enum STATE {
        PREINIT,
        LOADING,
        MENU,

        PLAYING,
        DEAD,
        STOPPING,
    };
    void Init();
    void Run();

    void Capture() {
        if(!bCaptured){
            if(SDL_SetRelativeMouseMode(SDL_TRUE) == 0){
                SDL_ShowCursor(SDL_FALSE);
                bCaptured = true;
            }
        }
    }
    void Release()
    {
        if(bCaptured){
            if(SDL_SetRelativeMouseMode(SDL_FALSE) == 0){
                SDL_ShowCursor(SDL_TRUE);
                bCaptured = false;
            }
        }
    }
    float GetTime() { return (float)(SDL_GetPerformanceCounter() - uStartTime) / SDL_GetPerformanceFrequency(); }

private:
    STATE eState;
    bool bCaptured;
    uint64_t uStartTime;

    GPU* xGpu;
    Scene* scene;
    GUI* gui;
    Scene::SceneData sceneData;

    Player player;
    Sound* sfxLoop;
    uint32_t sfxLoopHandle;

    uint64_t uFrame;
    float fps, fpsLastTime, fMenuTime;
    float fLastFrameTime, fFrameTime, fFrameTimes[100];

    void Frame();
};
