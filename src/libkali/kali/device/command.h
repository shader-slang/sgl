#pragma once

#include "kali/device/fwd.h"
#include "kali/device/native_handle.h"
#include "kali/device/resource.h"
#include "kali/device/fence.h"
#include "kali/device/shader_cursor2.h"
#include "kali/device/shader_object.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <slang-gfx.h>

#include <functional>

namespace gfx {
struct ShaderCursor;
};

namespace kali {

enum CommandQueueType {
    graphics = gfx::ICommandQueue::QueueType::Graphics,
};

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
struct CommandStreamDesc {
    /// Queue type to use for the command stream.
    CommandQueueType queue_type{CommandQueueType::graphics};

    /// Maximum number of "in-flight" frames.
    /// This determines the number of resource heaps to use internally.
    /// During rendering, we typically need at least two frames, one being rendered to, the other being presented.
    /// We add one more to be on the save side.
    uint32_t frame_count{3};
};

class KALI_API CommandStream : public Object {
    KALI_OBJECT(CommandStream)
public:
    using SetShaderVariablesCallback = std::function<void(gfx::ShaderCursor)>;

    CommandStream(ref<Device> device, CommandStreamDesc desc);

    Device* get_device() const { return m_device; }
    CommandQueue* get_command_queue() const { return m_command_queue; }

    void submit();

    // ------------------------------------------------------------------------
    // Synchronization
    // ------------------------------------------------------------------------

    /**
     * Signal a fence.
     * @param pFence The fence to signal.
     * @param value The value to signal. If @c Fence::AUTO, the signaled value will be auto-incremented.
     * @return The signaled value.
     */
    uint64_t signal(Fence* fence, uint64_t value = Fence::AUTO);

    /**
     * Wait for a fence to be signaled on the device.
     * Queues a device-side wait and returns immediately.
     * The device will wait until the fence reaches or exceeds the specified value.
     * @param pFence The fence to wait for.
     * @param value The value to wait for. If @c Fence::AUTO, wait for the last signaled value.
     */
    void wait(Fence* fence, uint64_t value = Fence::AUTO);

    // ------------------------------------------------------------------------
    // Barriers
    // ------------------------------------------------------------------------

    /**
     * Transition resource state of a buffer and add a barrier if state has changed.
     * @param buffer Buffer
     * @param new_state New state
     * @return True if barrier was recorded (i.e. state has changed).
     */
    bool buffer_barrier(const Buffer* buffer, ResourceState new_state);

    /**
     * Transition resource state of a texture and add a barrier if state has changed.
     * @param texture Texture
     * @param new_state New state
     * @return True if barrier was recorded (i.e. state has changed).
     */
    bool texture_barrier(const Texture* texture, ResourceState new_state);

    bool resource_barrier(const Resource* resource, ResourceState new_state);

#if 0
    /**
     * Insert a resource barrier
     * if pViewInfo is nullptr, will transition the entire resource. Otherwise, it will only transition the subresource
     * in the view
     * @return true if a barrier commands were recorded for the entire resource-view, otherwise false (for example, when
     * the current resource state is the same as the new state or when only some subresources were transitioned)
     */
    bool resource_barrier(
        const Resource* resource, ResourceState new_state, const ResourceViewInfo* pViewInfo = nullptr);
#endif

    /**
     * Insert a UAV barrier
     */
    void uav_barrier(const Resource* resource);

    // ------------------------------------------------------------------------
    // Resource copying
    // ------------------------------------------------------------------------

    /**
     * Copy an entire resource
     */
    void copy_resource(const Resource* dst, const Resource* src);

    /**
     * Copy a subresource.
     */
    void copy_subresource(const Texture* dst, uint32_t dst_sub_index, const Texture* src, uint32_t src_sub_index);

    /**
     * Copy part of a buffer.
     * @param dst Destination buffer
     * @param dst_offset Destination offset in bytes
     * @param src Source buffer
     * @param src_offset Source offset in bytes
     * @param size Size in bytes
     */
    void copy_buffer_region(
        const Buffer* dst,
        DeviceOffset dst_offset,
        const Buffer* src,
        DeviceOffset src_offset,
        DeviceSize size
    );

    void copy_texture_region(
        const Texture* dst,
        uint32_t dst_sub_index,
        const Texture* src,
        uint32_t src_sub_index,
        uint3 dst_offset,
        uint3 src_offset,
        uint3 size
    );


#if 0
    /**
     * Copy a region of a subresource from one texture to another
     * `srcOffset`, `dstOffset` and `size` describe the source and destination regions. For any channel of `extent` that
     * is -1, the source texture dimension will be used
     */
    void copySubresourceRegion(
        const Texture* pDst,
        uint32_t dstSubresource,
        const Texture* pSrc,
        uint32_t srcSubresource,
        const uint3& dstOffset = uint3(0),
        const uint3& srcOffset = uint3(0),
        const uint3& size = uint3(-1)
    );

    /**
     * Update a texture's subresource data
     * `offset` and `size` describe a region to update. For any channel of `extent` that is -1, the texture dimension
     * will be used. pData can't be null. The size of the pointed buffer must be equal to a single texel size times the
     * size of the region we are updating
     */
    void updateSubresourceData(
        const Texture* pDst,
        uint32_t subresource,
        const void* pData,
        const uint3& offset = uint3(0),
        const uint3& size = uint3(-1)
    );

    /**
     * Update an entire texture
     */
    void updateTextureData(const Texture* pTexture, const void* pData);

    /**
     * Update a buffer
     */
    void updateBuffer(const Buffer* pBuffer, const void* pData, size_t offset = 0, size_t numBytes = 0);

    /**
     * Read texture data synchronously. Calling this command will flush the pipeline and wait for the GPU to finish
     * execution
     */
    std::vector<uint8_t> readTextureSubresource(const Texture* pTexture, uint32_t subresourceIndex);
#endif

    void upload_buffer_data(const Buffer* buffer, size_t offset, size_t size, const void* data);

    void upload_texture_data(const Texture* texture, const void* data);

    void upload_texture_subresource_data(
        const Texture* texture,
        uint32_t subresource_index,
        uint32_t subresource_count,
        const void* data,
        uint3 offset = uint3{0},
        uint3 size = uint3{0xffffffff}
    );

    // ------------------------------------------------------------------------
    // Compute
    // ------------------------------------------------------------------------

    ref<ShaderObject> bind_compute_pipeline(const ComputePipelineState* pipeline);
    void bind_compute_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object);
    void dispatch_compute(uint3 thread_group_count);
    void dispatch_compute_indirect(Buffer* cmd_buffer, DeviceOffset offset);

    void dispatch_compute(
        ShaderProgram* program,
        uint3 thread_count,
        SetShaderVariablesCallback set_vars,
        ComputePipelineCache* pipeline_cache = nullptr
    );
    void dispatch_compute_indirect(
        ShaderProgram* program,
        SetShaderVariablesCallback set_vars,
        Buffer* cmd_buffer,
        DeviceOffset offset,
        ComputePipelineCache* pipeline_cache = nullptr
    );

    // ------------------------------------------------------------------------
    // Graphics
    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    // Raytracing
    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    // Debug events
    // ------------------------------------------------------------------------

    /// Begin a debug event.
    void begin_debug_event(const char* name, float3 color);

    /// End a debug event.
    void end_debug_event();

    void break_strong_reference_to_device();

private:
    void mark_pending();

    enum class EncoderType { none, resource, compute, render, raytracing };

    gfx::ITransientResourceHeap* get_current_transient_resource_heap() const
    {
        return m_frame_data[m_current_frame_index].transient_resource_heap;
    }

    gfx::IResourceCommandEncoder* get_resource_command_encoder();
    gfx::IComputeCommandEncoder* get_compute_command_encoder();
    gfx::IRenderCommandEncoder* get_render_command_encoder();
    gfx::IRayTracingCommandEncoder* get_raytracing_command_encoder();
    gfx::ICommandEncoder* request_encoder(EncoderType type);

    breakable_ref<Device> m_device;
    CommandStreamDesc m_desc;
    ref<CommandQueue> m_command_queue;
    ref<ComputePipelineCache> m_compute_pipeline_cache;

    struct FrameData {
        Slang::ComPtr<gfx::ITransientResourceHeap> transient_resource_heap;
    };

    std::vector<FrameData> m_frame_data;
    uint32_t m_current_frame_index{0};

    Slang::ComPtr<gfx::ICommandBuffer> m_command_buffer;
    gfx::ICommandEncoder* m_encoder{nullptr};
    EncoderType m_encoder_type{EncoderType::none};
};

} // namespace kali
