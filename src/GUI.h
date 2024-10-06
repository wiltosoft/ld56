/*
 * GUI.h
 *
 * Created by miles
*/

#pragma once

#include <stb_truetype.h>
#include "Asset.h"


class GUI {
public:
    GUI();

    static bool Init();

    bool Write(float size, int32_t x, int32_t y, const char* text, Vector4 col = {1,1,1,1});
    float WriteWidth(float size, const char* text);
    bool WriteCentered(float size, int32_t y, const char *text, Vector4 col = {1,1,1,1}) { return GUI::Write(size, (GPU::GetSize().getX() - WriteWidth(size, text)) / 2, y, text, col); }
    bool WriteShadowed(float size, int32_t x, int32_t y, const char *text, Vector4 col = {1,1,1,1}) { return GUI::Write(size, x, y, text, col) && GUI::Write(size, x + size * 2, y + size * 2, text, {0,0,0,0.66}); }
    static wgpu::TextureView GetTextureView() { return textureView; }


private:
    static Asset* ttf;
    stbtt_bakedchar chardata[128];

    wgpu::TextureDescriptor textureDesc;
    wgpu::Texture texture;
    static wgpu::TextureView textureView;
    wgpu::Sampler sampler;

    bool CreateTextureView(uint32_t mipLevels = 1);
};
