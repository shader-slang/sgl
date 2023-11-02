#pragma once

#include "kali/device/fwd.h"
#include "kali/device/native_handle.h"

#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <slang-gfx.h>

namespace kali {

enum CommandQueueType { graphics };

KALI_ENUM_INFO(
    CommandQueueType,
    {
        {CommandQueueType::graphics, "graphics"},
    }
);
KALI_ENUM_REGISTER(CommandQueueType);

struct CommandQueueDesc {
    CommandQueueType type;

    std::string to_string() const
    {
        return fmt::format(
            "CommandQueueDesc(\n"
            "    type={}\n"
            ")",
            type
        );
    }
};

class KALI_API CommandQueue : public Object {
    KALI_OBJECT(CommandQueue)
public:
    /// Constructor.
    /// Do not use directly, instead use @c Device::create_command_queue.
    CommandQueue(ref<Device> device, CommandQueueDesc desc);

    const CommandQueueDesc& desc() const { return m_desc; }

    /// Returns the native API handle for the command queue:
    /// - D3D12: ID3D12CommandQueue*
    /// - Vulkan: VkQueue (Vulkan)
    NativeHandle get_native_handle() const;

    gfx::ICommandQueue* get_gfx_command_queue() const { return m_gfx_command_queue; }

    void break_strong_reference_to_device();

    std::string to_string() const override;

private:
    breakable_ref<Device> m_device;
    CommandQueueDesc m_desc;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_command_queue;
};

class CommandBuffer : public Object {
    KALI_OBJECT(CommandBuffer)
public:
};

} // namespace kali
