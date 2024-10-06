/*
 * GUI.cpp
 *
 * Created by miles
*/

#define STB_TRUETYPE_IMPLEMENTATION

#include "GUI.h"
#include "Mesh.h"

Asset* GUI::ttf = nullptr;
wgpu::TextureView GUI::textureView;
constexpr uint32_t TSIZE = 1024;
constexpr uint32_t RENDERSIZE = 64;


bool GUI::Init()
{
    ttf = Asset::Fetch("assets/fonts/Audiowide.ttf");

    return true;
}



GUI::GUI()
{
    auto pixels = (unsigned char*)malloc(TSIZE * TSIZE * 1);
    // We just bake this out quick and dirty using this lib, nice and big so we can scale a little bit :O
    stbtt_BakeFontBitmap((unsigned char*)ttf->BlockingGet(), 0, RENDERSIZE, pixels, TSIZE, TSIZE, 32, 127, chardata);

    uint32_t mipLevels = 0;
    for(auto w = TSIZE; w; w >>= 1) mipLevels++;

    textureDesc.size = {TSIZE, TSIZE, 1};
    textureDesc.mipLevelCount = mipLevels;
    textureDesc.sampleCount = 1;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.format = wgpu::TextureFormat::R8Unorm;
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    texture = GPU::GetDevice().CreateTexture(&textureDesc);

    wgpu::ImageCopyTexture destination;
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 };
    destination.aspect = wgpu::TextureAspect::All;

    wgpu::TextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = textureDesc.size.width;
    source.rowsPerImage = textureDesc.size.height;

    wgpu::Extent3D size = {TSIZE, TSIZE, 1};
    GPU::GetQueue().WriteTexture(&destination, pixels, TSIZE * TSIZE * 1, &source, &size);

    // Manually create mip[s for now (mainly so we can scale the font a bit)
    auto d = TSIZE;
    for(auto ml = 1; ml < mipLevels; ml++){
        d >>= 1;
        for(auto y = 0; y < d; y++)
            for(auto x = 0; x < d; x++)
                pixels[y * d + x] = (pixels[(2 * y * d) + 2 * x] + pixels[(2 * y * d) + 2 * x + 1] + pixels[((2 * y + 1) * d) + 2 * x] + pixels[((2 * y + 1) * d) + 2 * x + 1]) >> 2;

        wgpu::Extent3D size = {d, d, 1};
        destination.mipLevel = ml;
        GPU::GetQueue().WriteTexture(&destination, pixels, TSIZE * TSIZE * 1, &source, &size);
    }

    CreateTextureView();
}


bool GUI::CreateTextureView(uint32_t mipLevels)
{
    wgpu::TextureViewDescriptor textureViewDesc;
    textureViewDesc.aspect = wgpu::TextureAspect::All;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = mipLevels;
    textureViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDesc.format = textureDesc.format;
    textureView = texture.CreateView(&textureViewDesc);

    return !!textureView;
}


bool GUI::Write(float size, int32_t x, int32_t y, const char *text, Vector4 col)
{
    Mesh m;
    auto vb = m.GetVertexBuffer(strlen(text) * 6);
    auto sz = GPU::GetSize() / 2;
    float cx = x, cy = y;

    // The stb function is unhelpful for scaling so hacked this out instead
    for(; *text; text++) {
        auto& cd = chardata[*text - 32];
        float dy = (cd.y1 - cd.y0) * size;
        float dx = (cd.x1 - cd.x0) * size;
        auto xoff = cd.xoff * size;
        auto yoff = cd.yoff * size;
        vb[0].pos = {-1.0f + (cx + xoff) / sz[0], 1.0f - (cy + yoff) / sz[1], 0};
        vb[0].col = col;
        vb[0].norm = {(float)cd.x0 / TSIZE, (float)cd.y0 / TSIZE, 0};
        vb[1].pos = {vb[0].pos.getX(), 1.0f - (cy + yoff + dy) / sz[1], 0};
        vb[1].col = col;
        vb[1].norm = {(float)cd.x0 / TSIZE, (float)cd.y1 / TSIZE, 0};
        vb[2].pos = {-1.0f + (cx + xoff + dx) / sz[0], vb[0].pos.getY(), 0};
        vb[2].col = col;
        vb[2].norm = {(float)cd.x1 / TSIZE, (float)cd.y0 / TSIZE, 0};
        vb[3] = vb[2];
        vb[4] = vb[1];
        vb[5].pos = {vb[2].pos.getX(), vb[1].pos.getY(), 0};
        vb[5].col = col;
        vb[5].norm = {(float)cd.x1 / TSIZE, (float)cd.y1 / TSIZE, 0};

        cx += cd.xadvance * size;
        vb += 6;
    }

    GPU::Encoder().SetVertexBuffer(0, m.GetGpuBuffer());
    GPU::Encoder().Draw(m.GetVertexCount());

    return true;
}


float GUI::WriteWidth(float size, const char *text)
{
    float cx = 0, cy = 0;

    for (; *text; text++) {
        stbtt_aligned_quad cq;
        stbtt_GetBakedQuad(chardata, TSIZE, TSIZE, *text - 32, &cx, &cy, &cq, true);
    }

    return cx * size;
}


