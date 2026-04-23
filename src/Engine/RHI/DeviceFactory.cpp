#include "Device.h"
#include "OpenGL/GLDevice.h"

#ifndef __EMSCRIPTEN__
#include "Vulkan/VKDevice.h"
#include "D3D12/DX12Device.h"
#include "D3D11/DX11Device.h"
#endif

namespace MulanGeo::Engine {

std::unique_ptr<RHIDevice> RHIDevice::create(const DeviceCreateInfo& ci) {
    switch (ci.backend) {
#ifndef __EMSCRIPTEN__
    case GraphicsBackend::Vulkan:
        return std::make_unique<VKDevice>(ci);

    case GraphicsBackend::D3D12:
        return std::make_unique<DX12Device>(ci);

    case GraphicsBackend::D3D11:
        return std::make_unique<DX11Device>(ci);
#endif

    case GraphicsBackend::OpenGL: {
        auto dev = std::make_unique<GLDevice>(ci);
        if (!dev->isInitialized()) {
            std::fprintf(stderr, "[RHIDevice] GLDevice initialization failed\n");
            return nullptr;
        }
        return dev;
    }

    default:
        return nullptr;
    }
}

} // namespace MulanGeo::Engine
