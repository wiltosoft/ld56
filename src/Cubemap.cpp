/*
 * Cubemap.cpp
 *
 * Created by miles
*/

#include "Cubemap.h"
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Cubemap::Cubemap(const char *name)
{
    const char* dt[6] = {"right", "left", "top", "bottom", "front", "back"};

    textureDesc.size = {2048, 2048, 6};
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    texture = GPU::GetDevice().CreateTexture(&textureDesc);

    wgpu::ImageCopyTexture destination;
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 };
    destination.aspect = wgpu::TextureAspect::All;

    wgpu::TextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = 4 * textureDesc.size.width;
    source.rowsPerImage = textureDesc.size.height;

    for(auto i = 0; i < 6; i++) {
        file[i] = file[i] + "assets/cubemaps/" + name + "/" + dt[i] + ".png";
        asset[i] = new Asset(file[i].c_str());
        int width, height, channels;
        char* pixels = (char*)stbi_load_from_memory((stbi_uc*)asset[i]->BlockingGet(), asset[i]->GetSize(), &width, &height, &channels, 4);
        if(pixels == nullptr){
            _ERROR("Failed to load imge %s", file[i].c_str());
            continue;
        }

        destination.origin.z = i;
        wgpu::Extent3D size = {(uint32_t)width, (uint32_t)height, 1};
        GPU::GetQueue().WriteTexture(&destination, pixels, width * height * 4, &source, &size);
    }

    CreateTextureView();
    CreateSampler();
}


bool Cubemap::CreateTextureView()
{
    wgpu::TextureViewDescriptor textureViewDesc;
    textureViewDesc.aspect = wgpu::TextureAspect::All;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 6;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = wgpu::TextureViewDimension::Cube;
    textureViewDesc.format = textureDesc.format;
    textureView = texture.CreateView(&textureViewDesc);

    return !!textureView;
}

bool Cubemap::CreateSampler()
{
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
    samplerDesc.addressModeV = wgpu::AddressMode::ClampToEdge;
    samplerDesc.addressModeW = wgpu::AddressMode::ClampToEdge;
    samplerDesc.magFilter = wgpu::FilterMode::Linear;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 1.0f;
    samplerDesc.compare = wgpu::CompareFunction::Undefined;
    samplerDesc.maxAnisotropy = 16;
    sampler = GPU::GetDevice().CreateSampler(&samplerDesc);

    return !!sampler;
}