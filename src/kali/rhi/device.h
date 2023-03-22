#pragma once

#include "fwd.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

#include <filesystem>
#include <string>
#include <vector>

namespace kali {

class Window;

enum class DeviceType {
    automatic,
    d3d12,
    vulkan,
    cpu,
    cuda,
};

struct DeviceDesc {
    DeviceType type{DeviceType::automatic};
};

class KALI_API Device : public Object {
public:
    Device(const DeviceDesc& desc = DeviceDesc{});
    ~Device();

    ref<Swapchain> create_swapchain(const SwapchainDesc& desc, ref<Window> window);

    ref<Buffer> create_buffer(const BufferDesc& desc, void* init_data = nullptr);

    // ref<Buffer> create_raw_buffer();
    // ref<Buffer> create_structured_buffer();
    // ref<Buffer> create_typed_buffer();

    ref<Program> create_program(const ProgramDesc& desc);

    ref<Program> create_program(std::filesystem::path path, std::string entrypoint);

    ProgramManager& get_program_manager();

    gfx::IDevice* get_gfx_device() const { return m_gfx_device.get(); }
    gfx::ICommandQueue* get_gfx_queue() const { return m_gfx_queue.get(); }

private:
    Slang::ComPtr<gfx::IDevice> m_gfx_device;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_queue;
    Slang::ComPtr<slang::IGlobalSession> m_slang_session;

    ref<ProgramManager> m_program_manager;
};

} // namespace kali
