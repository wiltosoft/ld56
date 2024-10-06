/*
 * Game.cpp
 *
 * Created by miles
*/

#include "Game.h"
#include "GPU.h"
#include "Scene.h"
#include "Sound.h"

#include <algorithm>


const float WARN_DIST = 3000;
const float DEATH_DIST = 8000;


void Game::Init()
{
    eState = LOADING;

    SDL_Init(SDL_INIT_EVERYTHING);
    uStartTime = SDL_GetPerformanceCounter();
    bCaptured = false;
    uFrame = 0;

    GPU::Init();
    GUI::Init();
    Scene::Init();
    Material::Init();
    Sound::Init();
    GPU::CreatePipeline();

    sfxLoop = new Sound("loop");
}


void Game::Run()
{
    SDL_Event event;
    eState = LOADING;

    gui = new GUI();
    scene = new Scene(&player);
    sceneData.fade = 1.0;

    fLastFrameTime = GetTime();


#ifdef EMSCRIPTEN
    emscripten_request_animation_frame_loop([](double t, void* userData)
                                            {
                                                auto game = (Game*)userData;
                                                game->Frame();
                                                return game->eState < STOPPING;
                                            }, this);
#endif
    while(eState < STOPPING) {
        while (SDL_PollEvent(&event)) {
            switch(event.type){
                case SDL_QUIT:
                    eState = STOPPING;
                    break;
                case SDL_WINDOWEVENT:
                    switch(event.window.event){
                        case SDL_WINDOWEVENT_RESIZED:
                            GPU::Resize(event.window.data1, event.window.data2);
                            break;
                        case SDL_WINDOWEVENT_CLOSE:
                            eState = STOPPING;
                            break;
                    }
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym){
                        case SDLK_w:
                            if(eState == PLAYING) player.propel.setY(1);
                            break;
                        case SDLK_s:
                            if(eState == PLAYING) player.propel.setY(-1);
                            break;
                        case SDLK_a:
                            if(eState == PLAYING) player.propel.setX(-1);
                            break;
                        case SDLK_d:
                            if(eState == PLAYING) player.propel.setX(1);
                            break;
                        case SDLK_SPACE:
                            if(eState == MENU) { eState = PLAYING; sceneData.fade = 1; }
                            if(eState == PLAYING) player.propel.setZ(1);
                        case SDLK_LCTRL:
                            if(eState == PLAYING) player.brake = true;
                            break;
                        case SDLK_ESCAPE:
                            Release();
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch(event.key.keysym.sym){
                        case SDLK_w:
                            if(eState == PLAYING) player.propel.setZ(0);
                            break;
                        case SDLK_s:
                            if(eState == PLAYING) player.propel.setZ(0);
                            break;
                        case SDLK_a:
                            if(eState == PLAYING) player.propel.setX(0);
                            break;
                        case SDLK_d:
                            if(eState == PLAYING) player.propel.setX(0);
                            break;
                        case SDLK_SPACE:
                            if(eState == PLAYING) player.propel.setZ(0);
                            break;
                        case SDLK_LCTRL:
                            if(eState == PLAYING) player.brake = false;
                            break;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if(!bCaptured && event.button.button == SDL_BUTTON_LEFT) Capture();
                    if(bCaptured && event.button.button == SDL_BUTTON_RIGHT) Release();
                    break;
                case SDL_MOUSEMOTION:
                    if(bCaptured && eState == PLAYING){
                        static constexpr float CLOSEST_ANGLE = 0.05;
                        auto view = player.camView;
                        auto axis = cross(normalize(view), player.up);
                        auto angle = acos(dot(normalize(view), player.up));
                        auto da = std::clamp(angle + ((float)event.motion.yrel / 1000), CLOSEST_ANGLE, 3.141592636f - CLOSEST_ANGLE) - angle;
                        view = Matrix3::rotationY((float)-event.motion.xrel / 1000) * view;
                        view = Matrix3::rotation(-da, axis) * view;
                        player.camView = view;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    if(bCaptured && eState == PLAYING){
                        auto view = player.camView;
                        view *= pow(1.1, -event.wheel.preciseY);
                        if(length(view) < player.scale) break;
                        player.camView = view;
                    }
                    break;
            }
        }

#ifndef EMSCRIPTEN
        Frame();
#endif

        if(Asset::Update() == 0 && eState == LOADING){
            eState = MENU;
            sfxLoopHandle = Sound::Loop(sfxLoop, 4);
            sceneData.fade = 0;
            fMenuTime = GetTime();
        }
        SLEEP(0);
    }
}


void Game::Frame()
{
    ++uFrame;

    fFrameTime = GetTime();
    float dt = fFrameTime - fLastFrameTime;
    fFrameTimes[uFrame % 100] = dt;

    if(uFrame % 100 == 0){
        float tt = 0;
        for(auto i = 0; i < 100; i++) tt += fFrameTimes[i];
        _INFO("Frame %4llu, %3.1f FPS", uFrame, 100.0f / tt);
    }

    if(fFrameTime - fpsLastTime >= 0.1) {
        fps = 0;
        for (auto i = 0; i < 10; i++) fps += fFrameTimes[(100 + uFrame - i) % 100];
        fps = 10.0f / fps;
        fpsLastTime = fFrameTime;
    }

    sceneData.time = fFrameTime;


    if(eState == PLAYING)
        scene->Update(fFrameTime, dt);

    if(eState == MENU){
        player.camView = Matrix3::rotationY(-dt / 33) * player.camView;
        sceneData.camera = player.pos - player.camView;
        sceneData.camView = player.camView;
        sceneData.projectionMatrix = Matrix4::perspective(3.1415927f * 0.333f, GPU::GetAspect(), 0.5, 10000.0);
        sceneData.viewMatrix = Matrix4::lookAt(Point3(sceneData.camera), Point3(player.pos), player.up);
        if(fFrameTime - fMenuTime < 4){
            sceneData.fade = (fFrameTime - fMenuTime) / 4;
        }
    }
    else {
        sceneData.camera = player.pos - player.camView;
        sceneData.camView = player.camView;
        sceneData.projectionMatrix = Matrix4::perspective(3.1415927f * 0.333f, GPU::GetAspect(), 0.5, 10000.0);
        sceneData.viewMatrix = Matrix4::lookAt(Point3(sceneData.camera), Point3(player.pos), player.up);
    }
    scene->UpdateSceneData(&sceneData);

    GPU::BeginFrame();

    // Render scene
    if(eState == PLAYING || eState == DEAD) {
        GPU::UsePlainShader();
        scene->Render();
    }
    else {
        // Because of a very simple unified pipeline, we have some unused bindings
        // that need to be established. They do no harm but just a bit ugly.
        Material::Bind(0);
        scene->BindNeededBuffers();
    }

    // Render skybox
    if(eState >= MENU) {
        GPU::UseSkyboxShader();
        GPU::Encoder().Draw(3);
    }

    // Render GUI
    GPU::UseGuiShader();
    char tb[128];
    switch(eState){
        case LOADING:
            sprintf(tb, "Loading...", player.followers);
            gui->Write(1, 20, 40, tb);
            break;
        case MENU:
            gui->WriteCentered(2.5, GPU::GetSize().getY() * 0.33, "BLU", {0.66, 0.66, 1, 1});
            if((int)(fFrameTime * 3) & 1) {
                sprintf(tb, "Press SPACE to play", player.followers);
                gui->WriteCentered(1, GPU::GetSize().getY() / 2, tb);
            }
            gui->WriteCentered(0.5, GPU::GetSize().getY() * 0.66, "CONTROLS", {0.66, 0.66, 0.88, 1.0});
            gui->WriteCentered(0.33, GPU::GetSize().getY() * 0.66 + 40, "Movement W S A D", {0.66, 0.66, 0.88, 1.0});
            gui->WriteCentered(0.33, GPU::GetSize().getY() * 0.66 + 70, "Accelerate SPACE", {0.66, 0.66, 0.88, 1.0});
            gui->WriteCentered(0.33, GPU::GetSize().getY() * 0.66 + 100, "Brake LEFT CTRL", {0.66, 0.66, 0.88, 1.0});
            gui->WriteCentered(0.33, GPU::GetSize().getY() * 0.66 + 130, "Zoom MOUSE WHEEL", {0.66, 0.66, 0.88, 1.0});
            break;
        case PLAYING:
            sprintf(tb, "%03u Followers", player.followers);
            gui->WriteShadowed(0.5, 20, 40, tb);

            if(length(player.pos) > WARN_DIST) {
                if((int)(fFrameTime * 4) & 1)
                    gui->WriteCentered(1, GPU::GetSize().getY() * 2.0f / 3, "THE PORTAL IS TOO FAR AWAY", {1,0,0,1});

                sceneData.fade = std::max(std::min((DEATH_DIST - length(player.pos)) / (DEATH_DIST - WARN_DIST), 1.0f), 0.0f);
                Sound::Volume(sfxLoopHandle, sceneData.fade);

                if(length(player.pos) > DEATH_DIST){
                    eState = DEAD;
                }
            }
            break;
        case DEAD:
            gui->WriteCentered(2, GPU::GetSize().getY() * 2.0f / 3, "YOU DIED =(", {1,0,0,1});
            break;
    }

    sprintf(tb, "%3.1f FPS", fps);
    auto tx = GPU::GetSize().getX() - 10 - gui->WriteWidth(0.3, tb);
    gui->WriteShadowed(0.3, tx, 20, tb);

    GPU::CompleteFrame();

    fLastFrameTime = fFrameTime;
}
