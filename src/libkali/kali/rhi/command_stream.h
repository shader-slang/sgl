#pragma once

#include "kali/rhi/fwd.h"
#include "kali/rhi/command_queue.h"
#include "kali/rhi/resource.h"
#include "kali/rhi/fence.h"
#include "kali/rhi/shader_object.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"

#include <functional>

namespace gfx {
struct ShaderCursor;
}

namespace kali {

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

    CommandStream(CommandStreamDesc desc, ref<Device> device);

    Device* get_device() const { return m_device; }
    CommandQueue* get_command_queue() const { return m_command_queue; }

    void submit();

    // ------------------------------------------------------------------------
    // Synchronization
    // ------------------------------------------------------------------------

    void signal(Fence* fence, uint64_t value = Fence::AUTO);

    void wait_device(Fence* fence, uint64_t value = Fence::AUTO);

    void wait_host(Fence* fence, uint64_t value = Fence::AUTO, uint64_t timeout_ns = Fence::TIMEOUT_INFINITE);

    void wait_host();

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
        const uint3& dst_offset,
        const uint3& src_offset,
        const uint3& size
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

    ShaderObject bind_compute_pipeline(const ComputePipelineState* pipeline);
    // void bind_compute_pipeline(const ComputePipelineState* pipeline, ShaderObject* shader_object);
    void dispatch_compute(uint3 thread_group_count);
    void dispatch_compute_indirect(Buffer* cmd_buffer, DeviceOffset offset);

    void dispatch_compute(
        Program* program,
        uint3 thread_count,
        SetShaderVariablesCallback set_vars,
        ComputePipelineCache* pipeline_cache = nullptr
    );
    void dispatch_compute_indirect(
        Program* program,
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

    CommandStreamDesc m_desc;
    breakable_ref<Device> m_device;
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
