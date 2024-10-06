/*
 * Object.h
 *
 * Created by miles
*/

#pragma once

#include <string>
#include <map>

#include "Asset.h"
#include "Mesh.h"


class Object {
public:
    struct InstanceData {
        Matrix4 modelMatrix;
        Vector4 col;

        static wgpu::VertexBufferLayout GetLayout()
        {
            static std::vector<wgpu::VertexAttribute> instanceAttributes = {
                    {wgpu::VertexFormat::Float32x4, sizeof(Vector4) * 0, 3},
                    {wgpu::VertexFormat::Float32x4, sizeof(Vector4) * 1, 4},
                    {wgpu::VertexFormat::Float32x4, sizeof(Vector4) * 2, 5},
                    {wgpu::VertexFormat::Float32x4, sizeof(Vector4) * 3, 6},
                    {wgpu::VertexFormat::Float32x4, sizeof(Vector4) * 4, 7},
            };

            wgpu::VertexBufferLayout instanceBufferLayout = {
                    .arrayStride = sizeof(InstanceData),
                    .stepMode = wgpu::VertexStepMode::Instance,
                    .attributeCount = instanceAttributes.size(),
                    .attributes = instanceAttributes.data(),
            };

            return instanceBufferLayout;
        }
    };

    struct Instance {
        InstanceData data;
        bool follower;
        float scale;
        float angle;
        float turn;
        float lastSound;
        btRigidBody* rb;
    };


    Object(const char* model);

    void Render(uint32_t count = 1, uint32_t offset = 0);
    Mesh& GetMesh(uint32_t i) { return xMesh[i]; }

    static Object* Load(const char* model) {
        auto res = xObjects.try_emplace(model, model);
        return &res.first->second;
    }

private:
    std::string objFile, mtlFile;
    Asset *obj, *mtl;

    std::vector<Mesh> xMesh;

    static std::map<const char*,Object> xObjects;
};
