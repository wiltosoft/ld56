/*
 * Cubemap.h
 *
 * Created by miles
*/

#pragma once

#include "Asset.h"

#include <string>


class Cubemap {
public:
    Cubemap(const char* name);

    wgpu::TextureView GetTextureView() { return textureView; }
    wgpu::Sampler GetSampler() { return sampler; }

private:
    std::string file[6];
    Asset* asset[6];

    wgpu::TextureDescriptor textureDesc;
    wgpu::Texture texture;
    wgpu::TextureView textureView;
    wgpu::Sampler sampler;

    bool CreateTextureView();

    bool CreateSampler();
};
