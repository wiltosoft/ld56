/*
 * Material.cpp
 *
 * Created by miles
*/

#include "Material.h"

wgpu::BindGroupLayout Material::bindGroupLayout = {};
wgpu::BindGroup Material::bindGroup = nullptr;
wgpu::Buffer Material::materialDataBuffer = nullptr;

uint32_t Material::uMaterialIndex = 0;
uint32_t Material::bufferStride = 0;
void* Material::xMaterials = nullptr;


bool Material::Init()
{
    wgpu::BufferBindingLayout bufferLayout = {
            .nextInChain = nullptr,
            .type = wgpu::BufferBindingType::Uniform,
            .hasDynamicOffset = true,
            .minBindingSize = sizeof(MaterialData),
    };
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry[] = {
            {.nextInChain = nullptr, .binding = 0, .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment, .buffer = bufferLayout, .sampler = {}, .texture = {}, .storageTexture = {}},
    };
    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
            .nextInChain = nullptr,
            .label = "material bgld",
            .entryCount = 1,
            .entries = bindGroupLayoutEntry,
    };
    bindGroupLayout = GPU::GetDevice().CreateBindGroupLayout(&bindGroupLayoutDescriptor);


    const auto stride = GPU::GetDeviceLimits().minUniformBufferOffsetAlignment;
    bufferStride = stride * ceil((float)sizeof(MaterialData) / stride);
    xMaterials = malloc(bufferStride * MAX_MATERIALS);

    wgpu::BufferDescriptor materialDataBufferDesc = {};
    materialDataBufferDesc.size = bufferStride * MAX_MATERIALS;
    materialDataBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    materialDataBuffer = GPU::GetDevice().CreateBuffer(&materialDataBufferDesc);


    wgpu::BindGroupEntry bindGroupEntries[] = {
            {.nextInChain = nullptr, .binding = 0, .buffer = materialDataBuffer, .offset = 0, .size = sizeof(MaterialData), .sampler = nullptr, .textureView = nullptr},
    };
    wgpu::BindGroupDescriptor bindGroupDescriptor = {
            .nextInChain = nullptr,
            .label = "bgd",
            .layout = bindGroupLayout,
            .entryCount = 1,
            .entries = bindGroupEntries,
    };
    bindGroup = GPU::GetDevice().CreateBindGroup(&bindGroupDescriptor);



    return bindGroupLayout && materialDataBuffer;
}