/*
 * GPU.h
 *
 * Created by miles
*/

#pragma once

class GPU {
public:
    GPU();

    static void Init() { xGPU = new GPU(); }

    static Vector2 GetSize() { return Vector2{(float)xGPU->width, (float)xGPU->height}; }
    static float GetAspect() { return (float)xGPU->width / xGPU->height; }
    static void Resize(uint32_t width, uint32_t height);

    static wgpu::Instance& GetInstance() { return xGPU->instance; }
    static wgpu::Adapter& GetAdapter() { return xGPU->adapter; }
    static wgpu::Device& GetDevice() { return xGPU->device; }
    static wgpu::Surface& GetSurface() { return xGPU->surface; }
    static wgpu::Queue GetQueue() { return xGPU->device.GetQueue(); }
    static wgpu::RenderPassEncoder& Encoder() { return xGPU->renderPassEncoder; }
    static wgpu::Limits& GetDeviceLimits() { return xGPU->deviceLimits; }



    static bool CreatePipeline();
    static void BeginFrame();
    static void CompleteFrame();

    static void UsePlainShader() { GPU::Encoder().SetPipeline(xGPU->rpPlain); }
    static void UseSkyboxShader() { GPU::Encoder().SetPipeline(xGPU->rpSkybox); }
    static void UseGuiShader() { GPU::Encoder().SetPipeline(xGPU->rpGui); }

private:
    static GPU* xGPU;
    bool bInitialised;

    SDL_Window* window;
    uint32_t width = 1024, height = 768;

    wgpu::Instance instance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Queue queue;

    wgpu::Limits deviceLimits;

    wgpu::Texture depthTexture;
    wgpu::TextureView depthTextureView;

    wgpu::Surface surface;
    wgpu::SurfaceTexture surfaceTexture;
    wgpu::TextureView targetView;
    wgpu::SurfaceCapabilities surfaceCapabilities;
    wgpu::SurfaceDescriptor surfaceDescriptor;
#ifdef _WIN32
    wgpu::SurfaceDescriptorFromWindowsHWND surfaceDescriptorFrom;
#else
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector surfaceDescriptorFrom;
#endif

    wgpu::CommandEncoder commandEncoder;
    wgpu::RenderPassEncoder renderPassEncoder;

    wgpu::RenderPipeline rpPlain, rpSkybox, rpGui;
    wgpu::BindGroup instanceBindGroup;
    wgpu::DepthStencilState depthStencilState;


    bool InitWindow();
    bool InitInstance();
    bool InitAdapter();
    bool InitDevice();
    bool InitSurface();

    bool InitBuffers();
};
