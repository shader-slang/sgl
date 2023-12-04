#pragma once

#include "kali/device/fwd.h"
#include "kali/device/fence.h"
#include "kali/device/resource.h"
#include "kali/device/shader_object.h"

#include "kali/core/object.h"
#include "kali/math/vector_types.h"

#include <span>

namespace kali {

enum CommandQueueType : uint32_t {
    graphics = static_cast<uint32_t>(gfx::ICommandQueue::QueueType::Graphics),
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
};

class KALI_API CommandQueue : public DeviceResource {
    KALI_OBJECT(CommandQueue)
public:
    /// Constructor.
    /// Do not use directly, instead use @c Device::create_command_queue.
    CommandQueue(ref<Device> device, CommandQueueDesc desc);

    const CommandQueueDesc& desc() const { return m_desc; }

    void submit(const CommandBuffer* command_buffer);

    void wait();

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
    void wait(const Fence* fence, uint64_t value = Fence::AUTO);

    /// Returns the native API handle for the command queue:
    /// - D3D12: ID3D12CommandQueue*
    /// - Vulkan: VkQueue (Vulkan)
    NativeHandle get_native_handle() const;

    gfx::ICommandQueue* gfx_command_queue() const { return m_gfx_command_queue; }

    std::string to_string() const override;

private:
    CommandQueueDesc m_desc;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_command_queue;
};

class KALI_API CommandBuffer : public DeviceResource {
    KALI_OBJECT(CommandBuffer)
public:
    CommandBuffer(ref<Device> device, Slang::ComPtr<gfx::ICommandBuffer> gfx_command_buffer);

    gfx::ICommandBuffer* gfx_command_buffer() const { return m_gfx_command_buffer; }

    std::string to_string() const override;

private:
    Slang::ComPtr<gfx::ICommandBuffer> m_gfx_command_buffer;
};

class KALI_API ComputePassEncoder {
public:
    ComputePassEncoder(ComputePassEncoder&& other) noexcept
        : m_command_stream(std::exchange(other.m_command_stream, nullptr))
        , m_gfx_compute_command_encoder(std::exchange(other.m_gfx_compute_command_encoder, nullptr))
    {
    }

    ~ComputePassEncoder();

    void end();

    ref<TransientShaderObject> bind_pipeline(const ComputePipelineState* pipeline);
    void bind_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object);
    void dispatch(uint3 thread_count);
    void dispatch_thread_groups(uint3 thread_group_count);
    void dispatch_thread_groups_indirect(const Buffer* cmd_buffer, DeviceOffset offset);

private:
    ComputePassEncoder(CommandStream* command_stream, gfx::IComputeCommandEncoder* gfx_compute_command_encoder)
        : m_command_stream(command_stream)
        , m_gfx_compute_command_encoder(gfx_compute_command_encoder)
    {
    }

    ComputePassEncoder(const ComputePassEncoder&) = delete;
    ComputePassEncoder& operator=(const ComputePassEncoder&) = delete;
    ComputePassEncoder& operator=(ComputePassEncoder&& other) noexcept = delete;

    CommandStream* m_command_stream;
    gfx::IComputeCommandEncoder* m_gfx_compute_command_encoder;
    const ComputePipelineState* m_bound_pipeline{nullptr};

    friend class CommandStream;
};

#if 0
class KALI_API RenderPassEncoder : public PassEncoder {
public:
    ref<TransientShaderObject> bind_pipeline(const GraphicsPipelineState* pipeline);
    void bind_pipeline(const GraphicsPipelineState* pipeline, const ShaderObject* shader_object);

private:
    RenderPassEncoder(CommandStream* command_stream)
        : PassEncoder(command_stream)
    {
    }

    void begin();

    gfx::IRenderCommandEncoder* m_gfx_render_command_encoder;

    friend class CommandStream;
};

class KALI_API RayTracingPassEncoder : public PassEncoder {
public:
    ref<TransientShaderObject> bind_pipeline(const RayTracingPipelineState* pipeline);
    void bind_pipeline(const RayTracingPipelineState* pipeline, const ShaderObject* shader_object);
    void dispatch_rays(uint32_t ray_gen_shader_index, const ShaderTable* shader_table, uint3 dimensions);

private:
    RayTracingPassEncoder(CommandStream* command_stream)
        : PassEncoder(command_stream)
    {
    }

    void begin();

    gfx::IRayTracingCommandEncoder* m_gfx_ray_tracing_command_encoder;

    friend class CommandStream;
};
#endif
class KALI_API CommandStream : public DeviceResource {
    KALI_OBJECT(CommandStream)
public:
    CommandStream(ref<Device> device, ref<CommandQueue> command_queue);

    /**
     * Submit all recorded commands to the command queue.
     */
    void submit();

    // ------------------------------------------------------------------------
    // Fences
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
    void wait(const Fence* fence, uint64_t value = Fence::AUTO);

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

    bool texture_subresource_barrier(
        const Texture* texture,
        uint32_t array_slice,
        uint32_t mip_level,
        ResourceState new_state
    );

    bool resource_barrier(const Resource* resource, ResourceState new_state);

    /**
     * Insert a UAV barrier
     */
    void uav_barrier(const Resource* resource);

    /**
     * @brief Insert a barrier for a set of buffers.
     *
     * @param buffers List of buffers.
     * @param old_state The state the buffers must be in before the barrier.
     * @param new_state The state the buffers must be in after the barrier.
     */
    void buffer_barrier(std::span<const Buffer*> buffers, ResourceState old_state, ResourceState new_state);
    void buffer_barrier(const Buffer* buffer, ResourceState old_state, ResourceState new_state);

    // TODO: Barrier without state tracking
    void texture_barrier(std::span<const Texture*> textures, ResourceState old_state, ResourceState new_state);
    void texture_barrier(const Texture* texture, ResourceState old_state, ResourceState new_state);

    // TODO: Barrier without state tracking
    void texture_subresource_barrier(
        const Texture* texture,
        SubresourceRange subresource_range,
        ResourceState old_state,
        ResourceState new_state
    );

    // ------------------------------------------------------------------------
    // Resources
    // ------------------------------------------------------------------------

    /**
     * @brief Copy an entire resource.
     *
     * @param dst Destination resource.
     * @param src Source resource.
     */
    void copy_resource(Resource* dst, const Resource* src);

    /**
     * @brief Copy a buffer region.
     *
     * @param dst Destination buffer.
     * @param dst_offset Destination offset in bytes.
     * @param src Source buffer.
     * @param src_offset Source offset in bytes.
     * @param size Size in bytes.
     */
    void copy_buffer_region(
        Buffer* dst,
        DeviceOffset dst_offset,
        const Buffer* src,
        DeviceOffset src_offset,
        DeviceSize size
    );

    /**
     * @brief Copy a texture region.
     *
     * @param dst Destination texture.
     * @param dst_subresource Destination subresource index.
     * @param src Source texture.
     * @param src_subresource Source subresource index.
     * @param dst_offset Destination offset in texels.
     * @param src_offset Source offset in texels.
     * @param extent Size in texels (-1 for maximum possible size).
     */
    void copy_texture_region(
        Texture* dst,
        uint32_t dst_subresource,
        const Texture* src,
        uint32_t src_subresource,
        uint3 dst_offset = uint3(0),
        uint3 src_offset = uint3(0),
        uint3 extent = uint3(-1)
    );

    /**
     * @brief Copy a texture to a buffer.
     *
     * @param dst Destination buffer.
     * @param dst_offset Destination offset in bytes.
     * @param dst_size Destination size in bytes.
     * @param dst_row_stride Destination row stride in bytes.
     * @param src Source texture.
     * @param src_subresource Source subresource index.
     * @param src_offset Source offset in texels.
     * @param extent Extent in texels (-1 for maximum possible extent).
     */
    void copy_texture_to_buffer(
        Buffer* dst,
        DeviceOffset dst_offset,
        DeviceSize dst_size,
        DeviceSize dst_row_stride,
        const Texture* src,
        uint32_t src_subresource,
        uint3 src_offset = uint3(0),
        uint3 extent = uint3(-1)
    );

    /**
     * @brief Copy data from one buffer to another.
     *
     * @param dst Destination buffer.
     * @param dst_offset Offset into the destination buffer.
     * @param src Source buffer.
     * @param src_offset Offset into the source buffer.
     * @param size Number of bytes to copy.
     */
    void copy_buffer(Buffer* dst, DeviceOffset dst_offset, const Buffer* src, DeviceOffset src_offset, DeviceSize size);

    void upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data);


    /**
     * \brief Resolve a multi-sampled texture.
     *
     * Both \c dst and \c src must have the same dimensions, array-size, mip-count and format.
     * If any of these properties don't match, use \c resolve_subresource.
     *
     * \param dst Destination texture.
     * \param src Source texture.
     */
    void resolve_texture(Texture* dst, const Texture* src);

    /**
     * \brief Resolve a multi-sampled texture sub-resource.
     *
     * Both \c dst and \c src sub-resources must have the same dimensions and format.
     *
     * \param dst Destination texture.
     * \param dst_subresource Destination sub-resource index.
     * \param src Source texture.
     * \param src_subresource Source sub-resource index.
     */
    void resolve_subresource(Texture* dst, uint32_t dst_subresource, const Texture* src, uint32_t src_subresource);

    /**
     * @brief Begin a new compute pass.
     *
     * The returned \c ComputePassEncoder is used to bind compute pipelines and issue dispatches.
     * The compute pass is ended when the \c ComputePassEncoder is destroyed.
     */
    ComputePassEncoder begin_compute_pass();
    // RenderPassEncoder begin_render_pass(RenderPassLayout*, FrameBuffer*);
    // RayTracingPassEncoder begin_ray_tracing_pass();

    // ------------------------------------------------------------------------
    // Debug events
    // ------------------------------------------------------------------------

    /// Begin a debug event.
    void begin_debug_event(const char* name, float3 color);

    /// End a debug event.
    void end_debug_event();

    std::string to_string() const override;

private:
    /// Called by pass encoders when they are destroyed.
    void end_pass();

    /// Create the internal command buffer if it doesn't exist yet.
    void create_command_buffer();

    gfx::IResourceCommandEncoder* get_gfx_resource_command_encoder();
    void end_current_encoder();

    ref<CommandQueue> m_command_queue;
    ref<CommandBuffer> m_command_buffer;

    bool m_pass_open{false};

    enum class EncoderType {
        none,
        resource,
        compute,
        render,
        raytracing,
    };

    EncoderType m_active_encoder{EncoderType::none};
    Slang::ComPtr<gfx::ICommandEncoder> m_gfx_command_encoder;

    friend class ComputePassEncoder;
};

} // namespace kali
