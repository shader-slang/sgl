#pragma once

#include "fwd.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

namespace kali {

class Window;

class KALI_API Device : public Object {
public:
    Device();
    ~Device();

    ref<Swapchain> create_swapchain(const SwapchainDesc& desc, ref<Window> window);

    gfx::IDevice* get_gfx_device() const { return m_gfx_device.get(); }

private:
    Slang::ComPtr<gfx::IDevice> m_gfx_device;
};

} // namespace kali
