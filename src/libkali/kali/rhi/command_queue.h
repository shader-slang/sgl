#pragma once

#include "kali/rhi/fwd.h"
#include "kali/rhi/native_handle.h"

#include "kali/core/object.h"

#include <slang-gfx.h>

namespace kali {

enum CommandQueueType { graphics };

struct CommandQueueDesc {
    CommandQueueType type;
};

class CommandQueue : public Object {
    KALI_OBJECT(CommandQueue)
public:
    /// Constructor.
    /// Do not use directly, instead use @ref Device::create_command_queue.
    CommandQueue(CommandQueueDesc desc, ref<Device> device);

    const CommandQueueDesc& get_desc() const { return m_desc; }

    /// Returns the native API handle for the command queue:
    /// - D3D12: ID3D12CommandQueue*
    /// - Vulkan: VkQueue (Vulkan)
    NativeHandle get_native_handle() const;

    gfx::ICommandQueue* get_gfx_command_queue() const { return m_gfx_command_queue; }

    void break_strong_reference_to_device();

private:
    CommandQueueDesc m_desc;
    breakable_ref<Device> m_device;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_command_queue;
};

} // namespace kali
