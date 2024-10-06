/*
 * Scene.cpp
 *
 * Created by miles
*/

#include "Scene.h"
#include "GUI.h"

#include <random>

wgpu::BindGroupLayout Scene::bindGroupLayout;


bool Scene::Init()
{
    wgpu::BufferBindingLayout sceneBufferLayout = {
            .nextInChain = nullptr,
            .type = wgpu::BufferBindingType::Uniform,
            .hasDynamicOffset = false,
            .minBindingSize = sizeof(SceneData),
    };
    std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntry = {
            {.nextInChain = nullptr, .binding = 0, .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment, .buffer = sceneBufferLayout, .sampler = {}, .texture = {}, .storageTexture = {}},
            {.nextInChain = nullptr, .binding = 1, .visibility = wgpu::ShaderStage::Fragment, .buffer = {}, .sampler = {}, .texture = {.sampleType = wgpu::TextureSampleType::Float, .viewDimension = wgpu::TextureViewDimension::Cube}, .storageTexture = {}},
            {.nextInChain = nullptr, .binding = 2, .visibility = wgpu::ShaderStage::Fragment, .buffer = {}, .sampler = {.type = wgpu::SamplerBindingType::Filtering}, .texture = {}, .storageTexture = {}},
            {.nextInChain = nullptr, .binding = 3, .visibility = wgpu::ShaderStage::Fragment, .buffer = {}, .sampler = {}, .texture = {.sampleType = wgpu::TextureSampleType::Float, .viewDimension = wgpu::TextureViewDimension::e2D}, .storageTexture = {}},
    };
    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
            .nextInChain = nullptr,
            .label = "bgld",
            .entryCount = bindGroupLayoutEntry.size(),
            .entries = bindGroupLayoutEntry.data(),
    };
    bindGroupLayout = GPU::GetDevice().CreateBindGroupLayout(&bindGroupLayoutDescriptor);


    return !!bindGroupLayout;
}


Scene::Scene(Player* player)
    : player(player)
{
    // LOAD OBJECTS
    xoBlu = Object::Load("blu");
    xoTiny = Object::Load("tiny");
    xoRing = Object::Load("ring");
    xoSkybox = new Cubemap("space");

    sfxBump1 = new Sound("bump1");

    // BUFFER MATERIALS
    Material::Buffer();

    // SCENE BUFFER
    wgpu::BufferDescriptor sceneDataBufferDesc = {};
    sceneDataBufferDesc.size = sizeof(Scene::SceneData);
    sceneDataBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    sceneDataBuffer = GPU::GetDevice().CreateBuffer(&sceneDataBufferDesc);


    std::vector<wgpu::BindGroupEntry> bindGroupEntries = {
            {.nextInChain = nullptr, .binding = 0, .buffer = sceneDataBuffer, .offset = 0, .size = sizeof(SceneData), .sampler = nullptr, .textureView = nullptr},
            {.nextInChain = nullptr, .binding = 1, .buffer = nullptr, .offset = 0, .size = 0, .sampler = nullptr, .textureView = xoSkybox->GetTextureView()},
            {.nextInChain = nullptr, .binding = 2, .buffer = nullptr, .offset = 0, .size = 0, .sampler = xoSkybox->GetSampler(), .textureView = nullptr},
            {.nextInChain = nullptr, .binding = 3, .buffer = nullptr, .offset = 0, .size = 0, .sampler = nullptr, .textureView = GUI::GetTextureView()},
    };
    wgpu::BindGroupDescriptor bindGroupDescriptor = {
            .nextInChain = nullptr,
            .label = "bgd",
            .layout = bindGroupLayout,
            .entryCount = bindGroupEntries.size(),
            .entries = bindGroupEntries.data(),
    };
    bindGroup = GPU::GetDevice().CreateBindGroup(&bindGroupDescriptor);

    // INSTANCE BUFFER
    wgpu::BufferDescriptor instanceDataBufferDesc = {};
    instanceDataBufferDesc.size = sizeof(instanceData);
    instanceDataBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    instanceDataBuffer = GPU::GetDevice().CreateBuffer(&instanceDataBufferDesc);

    // STRAT PHYSICS
    btCC = new btDefaultCollisionConfiguration();
    btCD = new btCollisionDispatcher(btCC);
    btBI = new btDbvtBroadphase();
    btSolver = new btSequentialImpulseConstraintSolver();
    btWorld = new btDiscreteDynamicsWorld(btCD, btBI, btSolver, btCC);
    btWorld->setGravity({0, 0, 0});

    // CREATE INSTANCES
    xInstances.try_emplace(xoBlu);
    xInstances.try_emplace(xoTiny);
    xInstances.try_emplace(xoRing);


    {
        ring = &xInstances[xoRing].emplace_back();
        ring->scale = 1;
        ring->data.modelMatrix = Matrix4::translation(Vector3(0, 0, 30));

        auto m = new btTriangleMesh();
        auto& mm = xoRing->GetMesh(0);
        for(auto i = 0; i < mm.GetVertexCount(); i += 3)
            m->addTriangle(_bt(mm.GetVertex(i).pos), _bt(mm.GetVertex(i+1).pos), _bt(mm.GetVertex(i+2).pos));
        auto shape = new btBvhTriangleMeshShape(m, true);

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(_bt(Vector3(0, 0, 30)));

        btDefaultMotionState* rbMotion = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0, rbMotion, shape);
        ring->rb = new btRigidBody(rbInfo);
        btWorld->addRigidBody(ring->rb);
    }

    {
        auto& instance = xInstances[xoBlu].emplace_back();
        instance.scale = player->scale;
        instance.angle = 0;
        instance.data.col = Vector4(hsl2rgb(0, 1, 0.5), 1);

        auto shape = new btSphereShape(player->scale);
        btScalar mass(2);
        btVector3 inertia(_bt(Vector3{0.1}));
        shape->calculateLocalInertia(mass, inertia);

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(_bt(player->pos));

        btDefaultMotionState* rbMotion = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, rbMotion, shape, inertia);
        instance.rb = new btRigidBody(rbInfo);
        instance.rb->setRestitution(0.2);
//        instance.rb->setFriction(1);
//        instance.rb->setDamping(0, 1);
        instance.rb->setLinearVelocity(_bt(player->vel));
        instance.rb->setActivationState(DISABLE_DEACTIVATION);

        btWorld->addRigidBody(instance.rb);
        instance.rb->setGravity({0,0,0});

        instance.data.modelMatrix = Matrix4::translation(player->pos) * Matrix4::scale(Vector3(instance.scale));
    }

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(-1, 1);
    std::uniform_real_distribution<float> col(0, 1);
    for(auto i = 0; i < 1000; i++) {

        auto& instance = xInstances[xoTiny].emplace_back();
        instance.scale = (dist(mt) + 2) / 2;
        instance.lastSound = 0;
        instance.data.col = Vector4(hsl2rgb(col(mt), 0.8, 0.6), 1);

        Vector3 pos = {dist(mt) * 500, dist(mt) * 500, dist(mt) * 500};
        auto shape = new btSphereShape(instance.scale * 2);
        btScalar mass(0.01);
        btVector3 inertia(_bt(Vector3{0.01}));
        shape->calculateLocalInertia(mass, inertia);

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(_bt(pos));

        btDefaultMotionState* rbMotion = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, rbMotion, shape, inertia);
        instance.rb = new btRigidBody(rbInfo);
        instance.rb->setRestitution(0.2);
//        instance.rb->setFriction(1);
        instance.rb->setDamping(0.1, 1);
        //instance.rb->applyImpulse(_bt(Vector3(dist(mt) * 2, dist(mt) * 2, dist(mt) * 1)), _bt(Vector3(0)));

        btWorld->addRigidBody(instance.rb);
        instance.rb->setGravity({0,0,0});

        instance.data.modelMatrix = Matrix4::translation(pos) * Matrix4::scale(Vector3(instance.scale));
    }


}


void Scene::Render()
{
    uint32_t i = 0;
    for(auto oi: xInstances){
        for(auto instance: oi.second)
            instanceData[i++] = instance.data;
    }
    GPU::GetQueue().WriteBuffer(instanceDataBuffer, 0, instanceData, sizeof(Object::InstanceData) * i);

    GPU::Encoder().SetBindGroup(0, bindGroup, 0);
    GPU::Encoder().SetVertexBuffer(1, instanceDataBuffer, 0);

    i = 0;
    for(auto oi: xInstances) {
        oi.first->Render(oi.second.size(), i);
        i += oi.second.size();
    }
}


void Scene::UpdateSceneData(Scene::SceneData* sceneData)
{
    GPU::GetQueue().WriteBuffer(sceneDataBuffer, 0, sceneData, sizeof(Scene::SceneData));
}


void Scene::Update(float time, float dt)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(-1, 1);

    for(auto& b : xInstances[xoTiny]) {
        if(b.rb->getLinearVelocity().length() < 4 && !b.follower){
            b.rb->applyCentralImpulse(_bt(Vector3(dist(mt) * 1, dist(mt) * 1, 0)));
        }
    }


//    for(auto i = 0; i < coa.size(); i++){
//        if(coa[i[.rb->hasContactResponse() && b.lastSound < time - 0.5){
//            Sound::Play(sfxBump1, _vm(t.getOrigin()));
//            b.lastSound = time;
//        }
//    }
//    _INFO("Collisons? %d", coa.size());

    for(auto& b : xInstances[xoBlu]){
        auto t = b.rb->getWorldTransform();
        auto v = _vm(b.rb->getLinearVelocity());
        auto lv = length(v);
        auto playerView = Matrix3::rotationY(b.angle) * Vector3(-1, 0, 0);
        auto playerSide = cross(playerView, player->up);
        auto i = _bt((float)player->propel.getZ() * 10 * dt * playerView);
//alImpulse(_bt((float)player->propel.getX() * lv * dt * playerSide)
        b.rb->setLinearVelocity(_bt(Matrix3::rotation((float)player->propel.getX() * dt * 0.1, player->up) * v * (player->brake ? pow(0.5, dt) : 1)));
        b.rb->applyCentralImpulse(_bt((float)player->propel.getZ() * 10 * dt * player->camView));
    }
    player->followers = 0;
    for(auto& b : xInstances[xoTiny]){
        auto t = b.rb->getWorldTransform();
        auto v = player->pos - _vm(t.getOrigin());
        auto lv = length(v);
        if (lv < player->scale * 5) { if(!b.follower) Sound::Play(sfxBump1, {0,0,0.5},1); b.follower = true; }
        if (lv > player->scale * 20) b.follower = false;
        v *= 50 * dt / (lv * lv);
        if(b.follower) {
            player->followers++;
            b.rb->applyCentralImpulse(_bt(v));
        }
    }

    btWorld->stepSimulation(dt, 3);

    for(auto& b : xInstances[xoBlu]){
        auto t = b.rb->getWorldTransform();
        auto v = b.rb->getLinearVelocity();
        v.setY(0);
        auto angle = v.angle(btVector3(-1,0,0));
        if(!isnan(angle)) b.angle = angle;
        player->pos = _vm(t.getOrigin());
        b.data.modelMatrix = Matrix4::translation(player->pos) * Matrix4::scale(Vector3(b.scale)) * Matrix4::rotationY(b.angle);
    }

    for(auto& b : xInstances[xoTiny]){
        auto t = b.rb->getWorldTransform();
        auto v = b.rb->getLinearVelocity();
        v.setY(0);
        auto angle = v.angle(btVector3(-1,0,0));
        if(!isnan(angle)) b.angle = angle;
        b.data.modelMatrix = Matrix4::translation(_vm(t.getOrigin())) * Matrix4::scale(Vector3(b.scale)) * Matrix4::rotationY(b.angle);
    }
}
