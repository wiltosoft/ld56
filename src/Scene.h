/*
 * Scene.h
 *
 * Created by miles
*/

#pragma once

#include "global.h"
#include "Object.h"
#include "Cubemap.h"
#include "Player.h"
#include "Sound.h"


class Scene {
public:
    static constexpr uint32_t MAX_INSTANCES = 10000;

    struct alignas(Vector4) SceneData {
        Matrix4 projectionMatrix;
        Matrix4 viewMatrix;
        Vector4 color;
        Vector3 camera;
        Vector3 camView;
        float time;
        float fade;
    };


    Scene(Player* player);

    static bool Init();
    static wgpu::BindGroupLayout GetBindGroupLayout() { return bindGroupLayout; }

    void UpdateSceneData(Scene::SceneData* sceneData);
    void Render();
    void BindNeededBuffers()
    {
        GPU::Encoder().SetBindGroup(0, bindGroup, 0);
        GPU::Encoder().SetVertexBuffer(0, instanceDataBuffer, 0);
        GPU::Encoder().SetVertexBuffer(1, instanceDataBuffer, 0);
    }

    void Update(float time, float dt);

private:
    static wgpu::BindGroupLayout bindGroupLayout;
    wgpu::BindGroup bindGroup;
    wgpu::Buffer sceneDataBuffer;

    std::unordered_map<Object*,std::vector<Object::Instance>> xInstances;
    Object::InstanceData instanceData[MAX_INSTANCES];
    wgpu::Buffer instanceDataBuffer;

    Object* xoBlu;
    Object* xoTiny;
    Object* xoRing;
    Object::Instance* ring;
    Cubemap* xoSkybox;
    Player* player;
    Sound* sfxBump1;

    btCollisionConfiguration* btCC;
    btCollisionDispatcher* btCD;
    btDbvtBroadphase* btBI;
    btConstraintSolver* btSolver;
    btDiscreteDynamicsWorld* btWorld;
};
