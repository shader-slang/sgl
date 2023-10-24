#pragma once

#include "kali/device/fwd.h"
#include "kali/device/native_handle.h"

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
    /// Do not use directly, instead use @see Device::create_command_queue.
    CommandQueue(ref<Device> device, CommandQueueDesc desc);

    const CommandQueueDesc& get_desc() const { return m_desc; }

    /// Returns the native API handle for the command queue:
    /// - D3D12: ID3D12CommandQueue*
    /// - Vulkan: VkQueue (Vulkan)
    NativeHandle get_native_handle() const;

    gfx::ICommandQueue* get_gfx_command_queue() const { return m_gfx_command_queue; }

    void break_strong_reference_to_device();

private:
    breakable_ref<Device> m_device;
    CommandQueueDesc m_desc;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_command_queue;
};

} // namespace kali
