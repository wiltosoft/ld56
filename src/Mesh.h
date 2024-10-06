/*
 * Mesh.h
 *
 * Created by miles
*/

#pragma once

#include "global.h"

#include "Material.h"

class Mesh {
public:
    Mesh() : bGpuBufferValid(false), vertexCount(0), vertexData(nullptr), bufferSize(0), vertexBuffer(), material(0) {}

    struct Vertex {
        Vector3 pos;
        Vector3 norm;
        Vector4 col;

        static wgpu::VertexBufferLayout GetLayout()
        {
            static std::vector<wgpu::VertexAttribute> vertexAttributes = {
                    {wgpu::VertexFormat::Float32x3, 0,                      0},
                    {wgpu::VertexFormat::Float32x3, offsetof(Mesh::Vertex, norm), 1},
                    {wgpu::VertexFormat::Float32x4, offsetof(Mesh::Vertex, col),  2},
            };

            wgpu::VertexBufferLayout vertexBufferLayout = {
                    .arrayStride = sizeof(Mesh::Vertex),
                    .stepMode = wgpu::VertexStepMode::Vertex,
                    .attributeCount = vertexAttributes.size(),
                    .attributes = vertexAttributes.data(),
            };

            return vertexBufferLayout;
        }
    };

    Vertex* GetVertexBuffer(uint32_t numberOfVertices);
    wgpu::Buffer GetGpuBuffer();
    uint32_t GetVertexCount() const { return vertexCount; }
    uint64_t GetBufferSize() const { return bufferSize; }
    void SetMaterial(uint32_t material) { this->material = material; }
    void BindMaterial() { Material::Bind(material); }
    Vertex& GetVertex(uint32_t i) { return vertexData[i]; }

private:
    bool bGpuBufferValid;
    uint32_t vertexCount, material;
    uint64_t  bufferSize;
    Vertex* vertexData;
    wgpu::Buffer vertexBuffer;


};
