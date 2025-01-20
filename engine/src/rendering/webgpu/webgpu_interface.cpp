#include "webgpu_interface.h"
#include <ermy_log.h>

#include "../../graphics/canvas.h"

WGPUDevice gDevice = nullptr;
WGPUAdapter gAdapter = nullptr;
WGPUInstance gInstance = nullptr;
WGPUQueue gQueue = nullptr;
WGPUSurface gSurface = nullptr;
WGPUSwapChain  gSwapChain = nullptr;
WGPUTextureFormat gPrefferedBackbufferFormat = WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm;

void RequestDeviceCallback(WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * userdata)
{
    ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"device requested: %s", message);
    bool succeed = status == WGPURequestDeviceStatus::WGPURequestDeviceStatus_Success;

    ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"device status: %s ptr:%p", succeed ? "success" : "fail", device);

    if(succeed)
    {
        gDevice = device;
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"getting queue");
        gQueue = wgpuDeviceGetQueue(device);

        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"create swapchain");
        WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
        canvasDesc.selector = "canvas";
        canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;

        WGPUSurfaceDescriptor surfDesc{};
        surfDesc.nextInChain = &(canvasDesc.chain);
        gSurface = wgpuInstanceCreateSurface(gInstance,&surfDesc);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"surface created: %p", gSurface);

        gPrefferedBackbufferFormat = wgpuSurfaceGetPreferredFormat(gSurface,gAdapter);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"gPrefferedBackbufferFormat: %d", gPrefferedBackbufferFormat);

        WGPUSwapChainDescriptor scDesc{};
        scDesc.usage = WGPUTextureUsage::WGPUTextureUsage_RenderAttachment;
        scDesc.format = gPrefferedBackbufferFormat;
        scDesc.width = 1280;
        scDesc.height = 720;
        scDesc.presentMode = WGPUPresentMode::WGPUPresentMode_Fifo;

        gSwapChain = wgpuDeviceCreateSwapChain(device,gSurface, &scDesc);

        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"swapchain created: %p", gSwapChain);
    }
}

void RequestAdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * userdata)
{
    ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter requested: %s", message);
    bool succeed = status == WGPURequestAdapterStatus::WGPURequestAdapterStatus_Success;

    ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter status: %s ptr:%p", succeed ? "success" : "fail", adapter);
    if(succeed)
    {
        gAdapter = adapter;
        WGPUAdapterProperties props = {};
        wgpuAdapterGetProperties(adapter,&props);

        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter name: %s",props.name);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter compatibility : %d",props.compatibilityMode);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter architecture: %s",props.architecture);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter description: %s",props.driverDescription);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter vendor: %s",props.vendorName);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter backend: %d",props.backendType);
        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"adapter type: %d",props.adapterType);

        WGPUSupportedLimits limits;
        wgpuAdapterGetLimits(adapter, &limits);

        ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"max tex2D: %d",limits.limits.maxTextureDimension2D);

        WGPUDeviceDescriptor desc = {};

        wgpuAdapterRequestDevice(adapter, &desc, RequestDeviceCallback, userdata);
    }
}

void rendering::Initialize()
{
   ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose,"RENDERING","WEBGPU Initialize");
   
    WGPUInstanceDescriptor desc = {};
    gInstance = wgpuCreateInstance(nullptr);

    ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Verbose,"instance created: %p", gInstance);

    WGPURequestAdapterOptions options = {};
    options.powerPreference = WGPUPowerPreference_HighPerformance;
    options.compatibilityMode = false;
    options.backendType = WGPUBackendType_WebGPU;
    ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose,"RENDERING","before adapter request");
    wgpuInstanceRequestAdapter(gInstance,&options,RequestAdapterCallback,nullptr);
    ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose,"RENDERING","after adapter request");

}

void rendering::Shutdown()
{
    ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose,"RENDERING","WEBGPU Shutdown");

    //printf("RENDERING","WEBGPU Shutdown/n");
}

void rendering::Process()
{
    WGPUColor bgColor;
    bgColor.a = canvas::BgColor[0];
    bgColor.r = canvas::BgColor[1];
    bgColor.g = canvas::BgColor[2];
    bgColor.b = canvas::BgColor[3];

 if(gSwapChain != nullptr)
    {
        auto view = wgpuSwapChainGetCurrentTextureView(gSwapChain);

        WGPURenderPassColorAttachment attachment{};
        attachment.view = view;
        attachment.loadOp = WGPULoadOp::WGPULoadOp_Clear;
        attachment.storeOp = WGPUStoreOp::WGPUStoreOp_Store;
        attachment.clearValue = bgColor;
        attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        WGPURenderPassDescriptor renderpass{};
        renderpass.colorAttachmentCount = 1;
        renderpass.colorAttachments = &attachment;


        WGPUCommandBuffer commands;

        {
            WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(gDevice, nullptr);
            {
                WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderpass);

                //llri::context ctx{ static_cast<void*>(pass) };
                //engine::renderCallback(ctx);

                wgpuRenderPassEncoderEnd(pass);
            }
            commands = wgpuCommandEncoderFinish(encoder,nullptr);
        }

        wgpuQueueSubmit(gQueue,1,&commands);
    }
}