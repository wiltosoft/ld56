/*
 * global.h
 *
 * Created by miles
*/

#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>

#define SLEEP emscripten_sleep
#else
#define SLEEP SDL_Delay
#endif

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_mouse.h>

#include "error.h"

#include <webgpu/webgpu_cpp.h>

#include <vectormath.hpp>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>


#include "tiny_obj_loader.h"
#include "stb_image.h"


#include <iostream>


#include "GPU.h"



inline btVector3 _bt(Vector3 v) { return btVector3(v.getX(), v.getY(), v.getZ()); }
inline btVector4 _bt(Vector4 v) { return btVector4(v.getX(), v.getY(), v.getZ(), v.getW()); }

inline Vector3 _vm(btVector3 v) { return Vector3(v.getX(), v.getY(), v.getZ()); }

inline Vector3 hsl2rgb(float h, float s, float l)
{
    constexpr static float SIXTH = (1.0f / 6.0f);
    float c = (1 - fabs(l * 2 -1)) * s;
    float x = c * (1 - abs(fmod(h / (1.0f / 6.0f), 2) - 1));
    float m = l - c / 2;
    if(h < SIXTH) return {c, x, 0};
    if(h < 2 * SIXTH) return {x, c, 0};
    if(h < 3 * SIXTH) return {0, c, x};
    if(h < 4 * SIXTH) return {0, x, c};
    if(h < 5 * SIXTH) return {x, 0, c};
    return {c, 0, x};
}
