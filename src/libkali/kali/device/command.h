#pragma once

#include "kali/device/fwd.h"
#include "kali/device/types.h"
#include "kali/device/device_resource.h"
#include "kali/device/native_handle.h"
#include "kali/device/resource.h"
#include "kali/device/raytracing.h"
#include "kali/device/fence.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/shader_object.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include "kali/math/vector_types.h"

#include <slang-gfx.h>

#include <functional>
#include <span>

namespace gfx {
struct ShaderCursor;
};

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
    CommandStream(ref<Device> device, CommandStreamDesc desc);

    Device* device() const { return m_device; }
    CommandQueue* command_queue() const { return m_command_queue; }

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

    bool texture_subresource_barrier(const Texture* texture, uint32_t array_slice, uint32_t mip_level, ResourceState new_state);

    bool resource_barrier(const Resource* resource, ResourceState new_state);

    // bool resource_barrier(const Resource* resource, ResourceState new_state, ResourceView)

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
    void copy_resource(Resource* dst, const Resource* src);

    /**
     * Copy a subresource.
     */
    void copy_subresource(Texture* dst, uint32_t dst_sub_index, const Texture* src, uint32_t src_sub_index);

    /**
     * Copy part of a buffer.
     * @param dst Destination buffer
     * @param dst_offset Destination offset in bytes
     * @param src Source buffer
     * @param src_offset Source offset in bytes
     * @param size Size in bytes
     */
    void copy_buffer_region(
        Buffer* dst,
        DeviceOffset dst_offset,
        const Buffer* src,
        DeviceOffset src_offset,
        DeviceSize size
    );

    void copy_texture_region(
        Texture* dst,
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
        Texture* pDst,
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
        Texture* pDst,
        uint32_t subresource,
        const void* pData,
        const uint3& offset = uint3(0),
        const uint3& size = uint3(-1)
    );

    /**
     * Update an entire texture
     */
    void updateTextureData(Texture* pTexture, const void* pData);

    /**
     * Update a buffer
     */
    void updateBuffer(Buffer* pBuffer, const void* pData, size_t offset = 0, size_t numBytes = 0);

    /**
     * Read texture data synchronously. Calling this command will flush the pipeline and wait for the GPU to finish
     * execution
     */
    std::vector<uint8_t> readTextureSubresource(const Texture* pTexture, uint32_t subresourceIndex);
#endif

    void upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data);

    void upload_texture_data(Texture* texture, const void* data);

    void upload_texture_subresource_data(
        Texture* texture,
        uint32_t subresource_index,
        uint32_t subresource_count,
        const void* data,
        uint3 offset = uint3{0},
        uint3 size = uint3{0xffffffff}
    );

    // ------------------------------------------------------------------------
    // Compute
    // ------------------------------------------------------------------------

    ref<TransientShaderObject> bind_compute_pipeline(const ComputePipelineState* pipeline);
    void bind_compute_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object);
    void dispatch_compute(uint3 thread_group_count);
    void dispatch_compute_indirect(const Buffer* cmd_buffer, DeviceOffset offset);

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
    enum class EncoderType { none, resource, compute, render, raytracing };

    gfx::ITransientResourceHeap* get_current_transient_resource_heap() const
    {
        return m_frame_data[m_current_frame_index].transient_resource_heap;
    }

    ResourceCommandEncoder* get_resource_command_encoder();
    ComputeCommandEncoder* get_compute_command_encoder();
    RenderCommandEncoder* get_render_command_encoder();
    RayTracingCommandEncoder* get_raytracing_command_encoder();
    CommandEncoder* request_encoder(EncoderType type);

    breakable_ref<Device> m_device;
    CommandStreamDesc m_desc;
    ref<CommandQueue> m_command_queue;

    struct FrameData {
        Slang::ComPtr<gfx::ITransientResourceHeap> transient_resource_heap;
    };

    std::vector<FrameData> m_frame_data;
    uint32_t m_current_frame_index{0};

    ref<CommandBuffer> m_command_buffer;
    CommandEncoder* m_encoder{nullptr};
    EncoderType m_encoder_type{EncoderType::none};
};

class KALI_API CommandEncoder {
public:
    virtual void end();

    void write_timestamp(QueryPool* query_pool, uint32_t index);

protected:
    CommandEncoder(CommandBuffer* command_buffer);
    void begin(gfx::ICommandEncoder* gfx_command_encoder);

    CommandBuffer* m_command_buffer;
    gfx::ICommandEncoder* m_gfx_command_encoder;
};

class KALI_API ResourceCommandEncoder : public CommandEncoder {
public:
    // ------------------------------------------------------------------------
    // Barriers
    // ------------------------------------------------------------------------

    // TODO: Barrier without state tracking
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
     * @brief Copy data from one buffer to another.
     *
     * @param dst Destination buffer.
     * @param dst_offset Offset into the destination buffer.
     * @param src Source buffer.
     * @param src_offset Offset into the source buffer.
     * @param size Number of bytes to copy.
     */
    void copy_buffer(Buffer* dst, DeviceOffset dst_offset, const Buffer* src, DeviceOffset src_offset, DeviceSize size);

#if 0
    // void copy_texture(Texture* dst, )

    /// Copies texture from src to dst. If dstSubresource and srcSubresource has mipLevelCount = 0
    /// and layerCount = 0, the entire resource is being copied and dstOffset, srcOffset and extent
    /// arguments are ignored.
    virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(
        ITextureResource* dst,
        ResourceState dstState,
        SubresourceRange dstSubresource,
        ITextureResource::Offset3D dstOffset,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Extents extent
    ) = 0;
#endif

#if 0
    /// Copies texture to a buffer. Each row is aligned to kTexturePitchAlignment.
    virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(
        IBufferResource* dst,
        Offset dstOffset,
        Size dstSize,
        Size dstRowStride,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Extents extent
    ) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(
        ITextureResource* dst,
        SubresourceRange subResourceRange,
        ITextureResource::Offset3D offset,
        ITextureResource::Extents extent,
        ITextureResource::SubresourceData* subResourceData,
        GfxCount subResourceDataCount
    ) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL uploadBufferData(IBufferResource* dst, Offset offset, Size size, void* data)
        = 0;
#endif

    void upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data);

    // ------------------------------------------------------------------------
    // Debug events
    // ------------------------------------------------------------------------

    /// Begin a debug event.
    void begin_debug_event(const char* name, float3 color);

    /// End a debug event.
    void end_debug_event();

protected:
    ResourceCommandEncoder(CommandBuffer* command_buffer);

    gfx::IResourceCommandEncoder* gfx_resource_command_encoder() const
    {
        return static_cast<gfx::IResourceCommandEncoder*>(m_gfx_command_encoder);
    }

    friend class CommandBuffer;
};

class KALI_API ComputeCommandEncoder : public ResourceCommandEncoder {
public:
    ref<TransientShaderObject> bind_pipeline(const ComputePipelineState* pipeline);
    void bind_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object);
    void dispatch_compute(uint3 thread_group_count);
    void dispatch_compute_indirect(const Buffer* cmd_buffer, DeviceOffset offset);

protected:
    ComputeCommandEncoder(CommandBuffer* command_buffer);

    gfx::IComputeCommandEncoder* gfx_compute_command_encoder() const
    {
        return static_cast<gfx::IComputeCommandEncoder*>(m_gfx_command_encoder);
    }

    friend class CommandBuffer;
};

class KALI_API RenderCommandEncoder : public ResourceCommandEncoder {
public:
    ref<TransientShaderObject> bind_pipeline(const GraphicsPipelineState* pipeline);
    void bind_pipeline(const GraphicsPipelineState* pipeline, const ShaderObject* shader_object);

    struct Viewport {
        float2 origin{0.f, 0.f};
        float2 extent{0.f, 0.f};
        float2 zrange{0.f, 1.f};
    }; // TODO
    static_assert(
        (sizeof(Viewport) == sizeof(gfx::Viewport))
            && (offsetof(Viewport, origin.x) == offsetof(gfx::Viewport, originX))
            && (offsetof(Viewport, origin.y) == offsetof(gfx::Viewport, originY))
            && (offsetof(Viewport, extent.x) == offsetof(gfx::Viewport, extentX))
            && (offsetof(Viewport, extent.y) == offsetof(gfx::Viewport, extentY))
            && (offsetof(Viewport, zrange.x) == offsetof(gfx::Viewport, minZ))
            && (offsetof(Viewport, zrange.y) == offsetof(gfx::Viewport, maxZ)),
        "Viewport struct mismatch"
    );

    void set_viewports(std::span<Viewport> viewports);
    struct ScissorRect {
        int2 min{0, 0};
        int2 max{0, 0};
    }; // TODO
    static_assert(
        (sizeof(ScissorRect) == sizeof(gfx::ScissorRect))
            && (offsetof(ScissorRect, min.x) == offsetof(gfx::ScissorRect, minX))
            && (offsetof(ScissorRect, min.y) == offsetof(gfx::ScissorRect, minY))
            && (offsetof(ScissorRect, max.x) == offsetof(gfx::ScissorRect, maxX))
            && (offsetof(ScissorRect, max.y) == offsetof(gfx::ScissorRect, maxY)),
        "ScissorRect struct mismatch"
    );
    void set_scissor_rects(std::span<ScissorRect> scissor_rects);

    void set_viewport_and_scissor_rect(const Viewport& viewport);

    // void set_primitive_topology(PrimitiveTopology topology);

    void set_stencil_reference(uint32_t reference_value);

    // void set_sample_positions(
    //     gfx_count samples_per_pixel, gfx_count pixel_count, const sample_position* sample_positions) = 0;


    struct Slot {
        const Buffer* buffer;
        DeviceOffset offset;
    };
    void set_vertex_buffers(uint32_t start_slot, std::span<Slot> slots);
    void set_vertex_buffer(uint32_t slot, const Buffer* buffer, DeviceOffset offset = 0);
    void set_index_buffer(const Buffer* buffer, Format index_format, DeviceOffset offset = 0);

    void draw(uint32_t vertex_count, uint32_t start_vertex = 0);

    void draw_indexed(uint32_t index_count, uint32_t start_index = 0, uint32_t base_vertex = 0);

    void draw_instanced(
        uint32_t vertex_count,
        uint32_t instance_count,
        uint32_t start_vertex = 0,
        uint32_t start_instance = 0
    );

    void draw_indexed_instanced(
        uint32_t index_count,
        uint32_t instance_count,
        uint32_t start_index = 0,
        uint32_t base_vertex = 0,
        uint32_t start_instance = 0
    );

    void draw_indirect(
        uint32_t max_draw_count,
        const Buffer* arg_buffer,
        DeviceOffset arg_offset,
        const Buffer* count_buffer = nullptr,
        DeviceOffset count_offset = 0
    );

    void draw_indexed_indirect(
        uint32_t max_draw_count,
        const Buffer* arg_buffer,
        DeviceOffset arg_offset,
        const Buffer* count_buffer = nullptr,
        DeviceOffset count_offset = 0
    );

    void draw_mesh_tasks(uint32_t x, uint32_t y, uint32_t z);

protected:
    RenderCommandEncoder(CommandBuffer* command_buffer);

    gfx::IRenderCommandEncoder* gfx_render_command_encoder() const
    {
        return static_cast<gfx::IRenderCommandEncoder*>(m_gfx_command_encoder);
    }

    friend class CommandBuffer;
};

class KALI_API RayTracingCommandEncoder : public ResourceCommandEncoder {
public:
    ref<TransientShaderObject> bind_pipeline(const RayTracingPipelineState* pipeline);
    void bind_pipeline(const RayTracingPipelineState* pipeline, const ShaderObject* shader_object);
    void dispatch_rays(uint32_t ray_gen_shader_index, const ShaderTable* shader_table, uint3 dimensions);

    // void build_acceleration_structure(
    //     const AccelerationStructure::BuildDesc& desc,
    //     GfxCount propertyQueryCount,
    //     AccelerationStructureQueryDesc* queryDescs) = 0;

    // void copy_acceleration_structure(
    //     AccelerationStructure* dst,
    //     AccelerationStructure* src,
    //     AccelerationStructure::CopyMode mode);

    // virtual SLANG_NO_THROW void SLANG_MCALL queryAccelerationStructureProperties(
    //     GfxCount accelerationStructureCount,
    //     IAccelerationStructure* const* accelerationStructures,
    //     GfxCount queryCount,
    //     AccelerationStructureQueryDesc* queryDescs) = 0;

    void serialize_acceleration_structure(DeviceAddress dst, const AccelerationStructure* src);
    void deserialize_acceleration_structure(AccelerationStructure* dst, DeviceAddress src);

    // /// Issues a dispatch command to start ray tracing workload with a ray tracing pipeline.
    // /// `rayGenShaderIndex` specifies the index into the shader table that identifies the ray generation shader.
    // virtual SLANG_NO_THROW Result SLANG_MCALL
    // dispatchRays(GfxIndex rayGenShaderIndex, IShaderTable* shaderTable, GfxCount width, GfxCount height, GfxCount
    // depth)
    //     = 0;

protected:
    RayTracingCommandEncoder(CommandBuffer* command_buffer);

    gfx::IRayTracingCommandEncoder* gfx_ray_tracing_command_encoder() const
    {
        return static_cast<gfx::IRayTracingCommandEncoder*>(m_gfx_command_encoder);
    }

    friend class CommandBuffer;
};


class KALI_API CommandBuffer : public Object {
    KALI_OBJECT(CommandBuffer)
public:
    CommandBuffer(Slang::ComPtr<gfx::ICommandBuffer> gfx_command_buffer);

    ResourceCommandEncoder* encode_resource_commands();
    ComputeCommandEncoder* encode_compute_commands();
    // RenderCommandEncoder* encode_render_commands(RenderPassLayout* render_pass, Framebuffer* frame_buffer);
    RayTracingCommandEncoder* encode_ray_tracing_commands();

    gfx::ICommandBuffer* gfx_command_buffer() const { return m_gfx_command_buffer; }

private:
    Slang::ComPtr<gfx::ICommandBuffer> m_gfx_command_buffer;
    ResourceCommandEncoder m_resource_command_encoder;
    ComputeCommandEncoder m_compute_command_encoder;
    RenderCommandEncoder m_graphics_command_encoder;
    RayTracingCommandEncoder m_ray_tracing_command_encoder;
    CommandEncoder* m_current_encoder{nullptr};

    friend class CommandEncoder;
};

} // namespace kali
