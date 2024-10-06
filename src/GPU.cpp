/*
 * GPU.cpp
 *
 * Created by miles
*/

#include "GPU.h"
#include "Scene.h"
#include "Object.h"
#include "Mesh.h"
#include "Asset.h"


GPU* GPU::xGPU = nullptr;


GPU::GPU()
{
    bInitialised = InitWindow()
        && InitInstance()
        && InitAdapter()
        && InitDevice()
        && InitSurface()
        && InitBuffers();
}


bool GPU::InitWindow()
{
    window = SDL_CreateWindow("LD56", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);

    return (bool)window;
}


bool GPU::InitInstance()
{
    instance = wgpu::CreateInstance(nullptr);

    return (bool)instance;
}


bool GPU::InitAdapter()
{
    wgpu::RequestAdapterOptions options = {};
    options.compatibleSurface = surface;

    wgpu::RequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.nextInChain = nullptr;
    callbackInfo.mode = wgpu::CallbackMode::AllowProcessEvents;
    callbackInfo.callback = [](WGPURequestAdapterStatus status,
                               WGPUAdapter adapter, const char *message,
                               void *userdata) {
        if (status != WGPURequestAdapterStatus_Success) {
            _ERROR("Failed to get adapter: %s", message);
            return;
        }
        *static_cast<wgpu::Adapter *>(userdata) = wgpu::Adapter::Acquire(adapter);
    };
    callbackInfo.userdata = &adapter;
    adapter = nullptr;
    instance.RequestAdapter(&options, callbackInfo.callback, &adapter);
    while (adapter == nullptr) {
        std::cerr << "Waiting for adapter\n";
        SLEEP(10);
    }

    return true;
}


bool GPU::InitDevice()
{
    wgpu::ErrorCallback errorCallback = [](WGPUErrorType type, char const *message, void *userdata) {
        _GPUERR("Error %d, %s", type, message);
    };

    wgpu::DeviceLostCallback deviceLostCallback = [](WGPUDeviceLostReason reason, char const * message, void * userdata) {
        // We'll need to reinitialise if we can
        _GPUERR("DEVICE LOST - %s", message);
    };

    wgpu::RequiredLimits limits = {};
    limits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;

    wgpu::DeviceDescriptor deviceDescriptor;
    deviceDescriptor.nextInChain = nullptr;
    deviceDescriptor.requiredFeatureCount = 0;
    deviceDescriptor.requiredLimits = &limits;
    deviceDescriptor.defaultQueue.nextInChain = nullptr;
    deviceDescriptor.deviceLostCallback = deviceLostCallback;

    device = nullptr;

    adapter.RequestDevice(&deviceDescriptor, [](WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * userdata) {
        if (status != WGPURequestDeviceStatus_Success) {
            _GPUERR("Failed to get GPU device : %s", message);
        }
        *static_cast<wgpu::Device *>(userdata) = wgpu::Device::Acquire(device);
    }, &device);
    while (device == nullptr) {
        _INFO("Waiting for device!");
        SLEEP(10);
    }

    _INFO("Device acquired!");


    wgpu::SupportedLimits supportedLimits;
    device.GetLimits(&supportedLimits);
    deviceLimits = supportedLimits.limits;

    device.SetUncapturedErrorCallback(errorCallback, nullptr);

    return true;
}

bool GPU::InitSurface()
{
    if(!surface) {
#ifdef _WIN32
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        auto r = SDL_GetWindowWMInfo(window, &wmInfo);

        surfaceDescriptorFrom.nextInChain = nullptr;
        surfaceDescriptorFrom.hwnd = wmInfo.info.win.window;
        surfaceDescriptorFrom.hinstance = wmInfo.info.win.hinstance;
#elif defined(__EMSCRIPTEN__)
        surfaceDescriptorFrom.nextInChain = nullptr;
        surfaceDescriptorFrom.selector = "canvas";
#else
#error Unsupported Platform
#endif

        surfaceDescriptor.label = "mysurface";
        surfaceDescriptor.nextInChain = &surfaceDescriptorFrom;

        surface = instance.CreateSurface(&surfaceDescriptor);
    }

    surface.GetCapabilities(adapter, &surfaceCapabilities);
    wgpu::SurfaceConfiguration surfaceConfiguration = {nullptr, device, surfaceCapabilities.formats[0],
                                                       wgpu::TextureUsage::RenderAttachment, 0,
                                                       nullptr, wgpu::CompositeAlphaMode::Opaque, width, height,
                                                       wgpu::PresentMode::Fifo};

    surface.Configure(&surfaceConfiguration);

    return (bool)surface;
}


bool GPU::CreatePipeline()
{
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.nextInChain = nullptr;
    wgslDesc.code = (const char*)Asset::Fetch("assets/shaders/plain.wgsl")->BlockingGet();
    wgpu::ShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &wgslDesc;
    shaderDesc.label = "plain";
    wgpu::ShaderModule smPlain = GetDevice().CreateShaderModule(&shaderDesc);

    wgslDesc.code = (const char*)Asset::Fetch("assets/shaders/skybox.wgsl")->BlockingGet();
    shaderDesc.label = "skybox";
    wgpu::ShaderModule smSkybox = GetDevice().CreateShaderModule(&shaderDesc);

    wgslDesc.code = (const char*)Asset::Fetch("assets/shaders/gui.wgsl")->BlockingGet();
    shaderDesc.label = "gui";
    wgpu::ShaderModule smGui = GetDevice().CreateShaderModule(&shaderDesc);

    if (!smPlain || !smSkybox) return false;


    xGPU->depthStencilState = {
            .nextInChain = nullptr,
            .format = wgpu::TextureFormat::Depth24Plus,
            .depthWriteEnabled = true,
            .depthCompare = wgpu::CompareFunction::Less,
            .stencilReadMask = 0,
            .stencilWriteMask = 0,
    };


    std::vector<wgpu::BindGroupLayout> bindGroupLayouts = { Scene::GetBindGroupLayout(), Material::GetBindGroupLayout() };
    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
            .nextInChain = nullptr,
            .bindGroupLayoutCount = bindGroupLayouts.size(),
            .bindGroupLayouts = bindGroupLayouts.data(),
    };
    wgpu::PipelineLayout pipelineLayout = GetDevice().CreatePipelineLayout(&pipelineLayoutDescriptor);

    std::vector<wgpu::VertexBufferLayout> vertexBuffers = {Mesh::Vertex::GetLayout(), Object::InstanceData::GetLayout()};

    wgpu::VertexState vertexState = {
            .nextInChain = nullptr,
            .module = smPlain,
            .entryPoint = "vs_main",
            .bufferCount = vertexBuffers.size(),
            .buffers = vertexBuffers.data(),
    };
    wgpu::BlendState blendState = {
            .color = {wgpu::BlendOperation::Add, wgpu::BlendFactor::SrcAlpha, wgpu::BlendFactor::OneMinusSrcAlpha},
            .alpha = {wgpu::BlendOperation::Add, wgpu::BlendFactor::SrcAlpha, wgpu::BlendFactor::OneMinusSrcAlpha},
    };
    wgpu::ColorTargetState colorTargetState = {
            .nextInChain = nullptr,
            .format = wgpu::TextureFormat::BGRA8Unorm,
            .blend = &blendState,
            .writeMask = wgpu::ColorWriteMask::All,
    };
    wgpu::FragmentState fragmentState = {
            .nextInChain = nullptr,
            .module = smPlain,
            .entryPoint = "fs_main",
            .targetCount = 1,
            .targets = &colorTargetState,

    };
    wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
            .nextInChain = nullptr,
            .layout = pipelineLayout,
            .vertex = vertexState,
            .primitive = {
                    .nextInChain = nullptr,
                    .topology = wgpu::PrimitiveTopology::TriangleList,
                    .stripIndexFormat = wgpu::IndexFormat::Undefined,
                    .frontFace = wgpu::FrontFace::CCW,
                    .cullMode = wgpu::CullMode::Back,
            },
            .depthStencil = &xGPU->depthStencilState,
            .multisample = {
                    .nextInChain = nullptr,
                    .count = 1,
                    .mask = 0xFFFFFFFF,
                    .alphaToCoverageEnabled = false,
            },
            .fragment = &fragmentState,
    };
    xGPU->rpPlain = GetDevice().CreateRenderPipeline(&renderPipelineDescriptor);
    if(!xGPU->rpPlain) _FATAL("Failed to create render pipeline: plain");

    vertexState.module = smSkybox;
    renderPipelineDescriptor.vertex = vertexState;
    fragmentState.module = smSkybox;
    xGPU->rpSkybox = GetDevice().CreateRenderPipeline(&renderPipelineDescriptor);
    if(!xGPU->rpSkybox) _FATAL("Failed to create render pipeline: skybox");

    vertexState.module = smGui;
    renderPipelineDescriptor.vertex = vertexState;
    fragmentState.module = smGui;
    xGPU->rpGui = GetDevice().CreateRenderPipeline(&renderPipelineDescriptor);
    if(!xGPU->rpSkybox) _FATAL("Failed to create render pipeline: gui");

    return true;
}


void GPU::BeginFrame()
{
    xGPU->surface.GetCurrentTexture(&xGPU->surfaceTexture);

    wgpu::TextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = xGPU->surfaceTexture.texture.GetFormat();
    viewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = wgpu::TextureAspect::All;
    xGPU->targetView = xGPU->surfaceTexture.texture.CreateView(&viewDescriptor);

    xGPU->commandEncoder = xGPU->device.CreateCommandEncoder();

    std::vector<wgpu::RenderPassColorAttachment> colorAttachments = {
            {
                .view = xGPU->targetView,
                .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                .resolveTarget = nullptr,
                .loadOp = wgpu::LoadOp::Clear,
                .storeOp = wgpu::StoreOp::Store,
                .clearValue = wgpu::Color{0.0, 0, 0, 1.0},
            }
    };
    wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
            xGPU->depthTextureView, wgpu::LoadOp::Clear, wgpu::StoreOp::Store, 1.0f, false, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, 0, true
    };

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = colorAttachments.size();
    renderPassDesc.colorAttachments = colorAttachments.data();
    renderPassDesc.depthStencilAttachment = &renderPassDepthStencilAttachment;

    xGPU->renderPassEncoder = xGPU->commandEncoder.BeginRenderPass(&renderPassDesc);

    xGPU->renderPassEncoder.SetPipeline(xGPU->rpPlain);
}


void GPU::CompleteFrame()
{
    xGPU->renderPassEncoder.End();

    wgpu::CommandBufferDescriptor commandBufferDescriptor;
    wgpu::CommandBuffer commandBuffer = xGPU->commandEncoder.Finish(&commandBufferDescriptor);

    GetQueue().Submit(1, &commandBuffer);

#ifndef __EMSCRIPTEN__
    xGPU->surface.Present();
#endif
}


void GPU::Resize(uint32_t width, uint32_t height)
{
    xGPU->width = width;
    xGPU->height = height;
    _INFO("Resize to %d x %d, aspect %f", xGPU->width, xGPU->height, GPU::GetAspect());
    xGPU->InitSurface();
    xGPU->InitBuffers();
}


bool GPU::InitBuffers()
{
    auto tf = wgpu::TextureFormat::Depth24Plus;

    if(depthTexture)
        depthTexture.Destroy();

    wgpu::TextureDescriptor depthTextureDesc = {};
    depthTextureDesc.dimension = wgpu::TextureDimension::e2D;
    depthTextureDesc.format = tf;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {width, height, 1};
    depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = &tf;
    depthTexture = device.CreateTexture(&depthTextureDesc);

    wgpu::TextureViewDescriptor depthTextureViewDesc = {};
    depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    depthTextureViewDesc.format = tf;
    depthTextureView = depthTexture.CreateView(&depthTextureViewDesc);

    return !!depthTextureView;
}





