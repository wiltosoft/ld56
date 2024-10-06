cmake_minimum_required(VERSION 3.29)
project(LD56)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(modules)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIR})
include_directories(${TINYOBJLOADER_INCLUDE_DIR})
include_directories(modules/stb)
include_directories(modules/vectormath)
include_directories(modules/minivorbis)
include_directories(modules/bullet3/src)

if(EMSCRIPTEN)
    message(STATUS "Compiling with Emscripten ${EMSCRIPTEN_SYSROOT}")

    include_directories(BEFORE ${EMSCRIPTEN_SYSROOT}/include)

    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_SCAN_FOR_MODULES 0)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_WEBGPU -sASSERTIONS=1 -jsDWEBGPU_DEBUG=1 -sASYNCIFY -sFETCH -sALLOW_MEMORY_GROWTH --shell-file ${CMAKE_SOURCE_DIR}/assets/index.html")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/www)
else()
    message(STATUS "Compiling with Dawn")
    include_directories(${DAWN_INCLUDE_DIR})
endif()

add_executable(LD56 src/main.cpp
        src/GPU.cpp
        src/GPU.h
        src/global.h
        src/Game.cpp
        src/Game.h
        src/error.h
        src/Sound.cpp
        src/Sound.h
        src/Scene.cpp
        src/Scene.h
        src/Mesh.cpp
        src/Mesh.h
        src/Asset.cpp
        src/Asset.h
        src/Object.cpp
        src/Object.h
        src/Material.cpp
        src/Material.h
        src/Player.cpp
        src/Player.h
        src/sound.c
        src/Cubemap.cpp
        src/Cubemap.h
        src/GUI.cpp
        src/GUI.h)


target_precompile_headers(LD56 PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/global.h>")

target_link_libraries(LD56 SDL2::SDL2-static SDL2::SDL2main freetype tinyobjloader BulletDynamics BulletCollision LinearMath)


if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(LD56 PROPERTIES OUTPUT_NAME "index")
    target_link_libraries(LD56)
    add_custom_target(html DEPENDS assets/index.html)
    add_dependencies(LD56 html)
else()
    target_link_libraries(LD56 dawn::webgpu_dawn)
endif()
