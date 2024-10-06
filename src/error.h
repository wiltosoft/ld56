/*
 * error.h
 *
 * Created by miles
*/

#pragma once

#include <cstdio>
#include <cstdlib>


#define _LOG(level, stream, ...) { fprintf(stream, "[%9.3f] %5s: ", 1000.0 * SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency(), #level); fprintf(stream, __VA_ARGS__); fprintf(stream, "\n"); fflush(stream); }
#ifdef NDEBUG
#define _DEBUG(...)
#else
#define LOG_DEBUG(...) _LOG(DEBUG, stdout, __VA_ARGS__)
#endif
#define _INFO(...) _LOG(INFO, stdout, __VA_ARGS__)
#define _GPUERR(...) _LOG(GPU, stdout, __VA_ARGS__)
#define _ERROR(...) _LOG(ERROR, stdout, __VA_ARGS__)
#define _FATAL(...) { _LOG(FATAL, stdout, __VA_ARGS__); std::abort(); exit(1); }
