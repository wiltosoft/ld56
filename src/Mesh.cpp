/*
 * Mesh.cpp
 *
 * Created by miles
*/

#include "Mesh.h"


Mesh::Vertex* Mesh::GetVertexBuffer(uint32_t numberOfVertices)
{
    const auto requiredBufferSize = numberOfVertices * sizeof(Vertex);
    if(vertexData && bufferSize < requiredBufferSize) {
        free(vertexData);
        bufferSize = 0;
    }
    if(!vertexData) {
        vertexData = (Vertex*)malloc(requiredBufferSize);
        if(vertexData) bufferSize = requiredBufferSize;
    }

    vertexCount = numberOfVertices;

    return vertexData;
}


wgpu::Buffer Mesh::GetGpuBuffer()
{
    if(vertexBuffer){
        if(bGpuBufferValid)
            return vertexBuffer;

        vertexBuffer.Destroy();
    }

    wgpu::BufferDescriptor bufferDescriptor = {
            .nextInChain = nullptr,
            .label = "vb",
            .usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst,
            .size = bufferSize,
            .mappedAtCreation = false,
    };
    vertexBuffer = GPU::GetDevice().CreateBuffer(&bufferDescriptor);
    GPU::GetQueue().WriteBuffer(vertexBuffer, 0, vertexData, bufferSize);

    bGpuBufferValid = !!vertexBuffer;

    return vertexBuffer;
}


