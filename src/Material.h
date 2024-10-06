/*
 * Material.h
 *
 * Created by miles
*/

#pragma once

#include <array>

#include "GPU.h"


class Material {
public:
    static constexpr uint32_t MAX_MATERIALS = 200;

    struct MaterialData {
        Vector4 col;
        Vector3 emission;
        float roughness;
        float metalness;
    };

    static bool Init();
    static void Bind(uint32_t uMaterialIndex) {
        uint32_t offset = bufferStride * uMaterialIndex;
        GPU::Encoder().SetBindGroup(1, bindGroup, 1, &offset);
    }
    static void Buffer() { GPU::GetQueue().WriteBuffer(materialDataBuffer, 0, xMaterials, bufferStride * MAX_MATERIALS); }
    static uint32_t Create(MaterialData& materialData) { *GetMaterial(uMaterialIndex) = materialData; return uMaterialIndex++; }
    static MaterialData* GetMaterial(uint32_t uMaterialIndex) { return (MaterialData*)((char*)xMaterials + bufferStride * uMaterialIndex); }

    static wgpu::BindGroupLayout GetBindGroupLayout() { return bindGroupLayout; }

private:
    static wgpu::BindGroupLayout bindGroupLayout;
    static wgpu::BindGroup bindGroup;
    static wgpu::Buffer materialDataBuffer;

    static uint32_t uMaterialIndex, bufferStride;
    static void* xMaterials;
};
