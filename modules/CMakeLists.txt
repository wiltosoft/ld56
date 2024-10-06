set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)

set(SDL_LIBC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(SDL2)

if(EMSCRIPTEN)
else()
    SET(DAWN_FETCH_DEPENDENCIES ON)
    add_subdirectory(dawn)
endif()

add_subdirectory(freetype)
add_subdirectory(tinyobjloader)
add_subdirectory(bullet3)
