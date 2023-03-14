#pragma once

#include "fwd.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

namespace kali {

class Window;

enum class DeviceType {
    automatic,
    d3d12,
    vulkan,
};

struct DeviceDesc {
    DeviceType type{DeviceType::automatic};
};

class KALI_API Device : public Object {
public:
    Device(const DeviceDesc& desc = DeviceDesc{});
    ~Device();

    ref<Swapchain> create_swapchain(const SwapchainDesc& desc, ref<Window> window);

    gfx::IDevice* get_gfx_device() const { return m_gfx_device.get(); }
    gfx::ICommandQueue* get_gfx_queue() const { return m_gfx_queue.get(); }

private:
    Slang::ComPtr<gfx::IDevice> m_gfx_device;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_queue;
};

} // namespace kali
