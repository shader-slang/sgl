// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/fence.h"
#include "sgl/device/resource.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/raytracing.h"

#include "sgl/core/object.h"
#include "sgl/math/vector_types.h"

#include <span>

namespace sgl {

class SGL_API ComputeCommandEncoder {
public:
    ComputeCommandEncoder(ComputeCommandEncoder&& other) noexcept
        : m_command_buffer(std::exchange(other.m_command_buffer, nullptr))
        , m_gfx_compute_command_encoder(std::exchange(other.m_gfx_compute_command_encoder, nullptr))
    {
    }

    ~ComputeCommandEncoder();

    void end();

    ref<TransientShaderObject> bind_pipeline(const ComputePipeline* pipeline);
    void bind_pipeline(const ComputePipeline* pipeline, const ShaderObject* shader_object);
    void dispatch(uint3 thread_count);
    void dispatch_thread_groups(uint3 thread_group_count);
    void dispatch_thread_groups_indirect(const Buffer* cmd_buffer, DeviceOffset offset);

private:
    ComputeCommandEncoder(CommandBuffer* command_buffer, gfx::IComputeCommandEncoder* gfx_compute_command_encoder)
        : m_command_buffer(command_buffer)
        , m_gfx_compute_command_encoder(gfx_compute_command_encoder)
    {
    }

    ComputeCommandEncoder(const ComputeCommandEncoder&) = delete;
    ComputeCommandEncoder& operator=(const ComputeCommandEncoder&) = delete;
    ComputeCommandEncoder& operator=(ComputeCommandEncoder&& other) noexcept = delete;

    CommandBuffer* m_command_buffer;
    gfx::IComputeCommandEncoder* m_gfx_compute_command_encoder;
    const ComputePipeline* m_bound_pipeline{nullptr};
    ref<const ShaderObject> m_bound_shader_object;

    friend class CommandBuffer;
};

class SGL_API RenderCommandEncoder {
public:
    RenderCommandEncoder(RenderCommandEncoder&& other) noexcept
        : m_command_buffer(std::exchange(other.m_command_buffer, nullptr))
        , m_gfx_render_command_encoder(std::exchange(other.m_gfx_render_command_encoder, nullptr))
    {
    }

    ~RenderCommandEncoder();

    void end();

    ref<TransientShaderObject> bind_pipeline(const GraphicsPipeline* pipeline);
    void bind_pipeline(const GraphicsPipeline* pipeline, const ShaderObject* shader_object);

    void set_viewports(std::span<Viewport> viewports);
    void set_scissor_rects(std::span<ScissorRect> scissor_rects);
    void set_viewport_and_scissor_rect(const Viewport& viewport);

    void set_primitive_topology(PrimitiveTopology topology);

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

private:
    RenderCommandEncoder(CommandBuffer* command_buffer, gfx::IRenderCommandEncoder* gfx_render_command_encoder)
        : m_command_buffer(command_buffer)
        , m_gfx_render_command_encoder(gfx_render_command_encoder)
    {
    }

    RenderCommandEncoder(const RenderCommandEncoder&) = delete;
    RenderCommandEncoder& operator=(const RenderCommandEncoder&) = delete;
    RenderCommandEncoder& operator=(RenderCommandEncoder&& other) noexcept = delete;

    CommandBuffer* m_command_buffer;
    gfx::IRenderCommandEncoder* m_gfx_render_command_encoder;
    const GraphicsPipeline* m_bound_pipeline{nullptr};
    ref<const ShaderObject> m_bound_shader_object;

    friend class CommandBuffer;
};

class SGL_API RayTracingCommandEncoder {
public:
    RayTracingCommandEncoder(RayTracingCommandEncoder&& other) noexcept
        : m_command_buffer(std::exchange(other.m_command_buffer, nullptr))
        , m_gfx_ray_tracing_command_encoder(std::exchange(other.m_gfx_ray_tracing_command_encoder, nullptr))
    {
    }

    ~RayTracingCommandEncoder();

    void end();

    ref<TransientShaderObject> bind_pipeline(const RayTracingPipeline* pipeline);
    void bind_pipeline(const RayTracingPipeline* pipeline, const ShaderObject* shader_object);
    void dispatch_rays(uint32_t ray_gen_shader_index, const ShaderTable* shader_table, uint3 dimensions);

    void build_acceleration_structure(
        const AccelerationStructureBuildDesc& desc,
        std::span<AccelerationStructureQueryDesc> queries = std::span<AccelerationStructureQueryDesc>()
    );

    void copy_acceleration_structure(
        AccelerationStructure* dst,
        const AccelerationStructure* src,
        AccelerationStructureCopyMode mode
    );

    void query_acceleration_structure_properties(
        std::span<const AccelerationStructure*> acceleration_structures,
        std::span<AccelerationStructureQueryDesc> queries
    );

    void serialize_acceleration_structure(DeviceAddress dst, const AccelerationStructure* src);
    void deserialize_acceleration_structure(AccelerationStructure* dst, DeviceAddress src);

private:
    RayTracingCommandEncoder(
        CommandBuffer* command_buffer,
        gfx::IRayTracingCommandEncoder* gfx_ray_tracing_command_encoder
    )
        : m_command_buffer(command_buffer)
        , m_gfx_ray_tracing_command_encoder(gfx_ray_tracing_command_encoder)
    {
    }

    RayTracingCommandEncoder(const RayTracingCommandEncoder&) = delete;
    RayTracingCommandEncoder& operator=(const RayTracingCommandEncoder&) = delete;
    RayTracingCommandEncoder& operator=(RayTracingCommandEncoder&& other) noexcept = delete;

    CommandBuffer* m_command_buffer;
    gfx::IRayTracingCommandEncoder* m_gfx_ray_tracing_command_encoder;
    const RayTracingPipeline* m_bound_pipeline{nullptr};
    ref<const ShaderObject> m_bound_shader_object;

    friend class CommandBuffer;
};

class SGL_API CommandBuffer : public DeviceResource {
    SGL_OBJECT(CommandBuffer)
public:
    CommandBuffer(ref<Device> device);
    ~CommandBuffer();

    /**
     * \brief Open the command buffer for recording.
     *
     * No-op if command buffer is already open.
     *
     * \note Due to current limitations, only a single command buffer can be open at any given time.
     */
    void open();

    /**
     * \brief Close the command buffer.
     *
     * No-op if command buffer is already closed.
     */
    void close();

    /// True if the command buffer is open.
    bool is_open() const { return m_open; }

    /**
     * \brief Submit the command buffer to the device.
     *
     * The returned submission ID can be used to wait for the command buffer to complete.
     *
     * \param queue Command queue to submit to.
     * \return Submission ID.
     */
    uint64_t submit(CommandQueueType queue = CommandQueueType::graphics);

    // ------------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------------

    /**
     * \brief Write a timestamp.
     *
     * \param query_pool Query pool.
     * \param index Index of the query.
     */
    void write_timestamp(QueryPool* query_pool, uint32_t index);

    /**
     * \brief Resolve a list of queries and write the results to a buffer.
     *
     * \param query_pool Query pool.
     * \param index Index of the first query.
     * \param count Number of queries to resolve.
     * \param buffer Destination buffer.
     * \param offset Offset into the destination buffer.
     */
    void resolve_query(QueryPool* query_pool, uint32_t index, uint32_t count, Buffer* buffer, DeviceOffset offset);

    // ------------------------------------------------------------------------
    // Resource state tracking
    // ------------------------------------------------------------------------

    /**
     * Transition resource state of a resource and add a barrier if state has changed.
     * \param resource Resource
     * \param new_state New state
     * \return True if barrier was recorded (i.e. state has changed).
     */
    bool set_resource_state(const Resource* resource, ResourceState new_state);

    /**
     * Transition resource state of a resource and add a barrier if state has changed.
     * For buffer views, this will set the resource state of the entire buffer.
     * For texture views, this will set the resource state of all its sub-resources.
     * \param resource_view Resource view
     * \param new_state New state
     * \return True if barrier was recorded (i.e. state has changed).
     */
    bool set_resource_state(const ResourceView* resource_view, ResourceState new_state);

    /**
     * Transition resource state of a buffer and add a barrier if state has changed.
     * \param buffer Buffer
     * \param new_state New state
     * \return True if barrier was recorded (i.e. state has changed).
     */
    bool set_buffer_state(const Buffer* buffer, ResourceState new_state);

    /**
     * Transition resource state of a texture and add a barrier if state has changed.
     * \param texture Texture
     * \param new_state New state
     * \return True if barrier was recorded (i.e. state has changed).
     */
    bool set_texture_state(const Texture* texture, ResourceState new_state);

    /**
     * Transition resource state of texture sub-resources and add a barrier if state has changed.
     * \param texture Texture
     * \param range Subresource range
     * \param new_state New state
     * \return True if barrier was recorded (i.e. state has changed).
     */
    bool set_texture_subresource_state(const Texture* texture, SubresourceRange range, ResourceState new_state);

    /**
     * Insert a UAV barrier
     */
    void uav_barrier(const Resource* resource);

    // ------------------------------------------------------------------------
    // Resources
    // ------------------------------------------------------------------------

    void clear_resource_view(ResourceView* resource_view, float4 clear_value);

    void clear_resource_view(ResourceView* resource_view, uint4 clear_value);

    void clear_resource_view(
        ResourceView* resource_view,
        float depth_value,
        uint32_t stencil_value,
        bool clear_depth,
        bool clear_stencil
    );

    void clear_texture(Texture* texture, float4 clear_value);

    void clear_texture(Texture* texture, uint4 clear_value);

    /**
     * \brief Copy an entire resource.
     *
     * \param dst Destination resource.
     * \param src Source resource.
     */
    void copy_resource(Resource* dst, const Resource* src);

    /**
     * \brief Copy a buffer region.
     *
     * \param dst Destination buffer.
     * \param dst_offset Destination offset in bytes.
     * \param src Source buffer.
     * \param src_offset Source offset in bytes.
     * \param size Size in bytes.
     */
    void copy_buffer_region(
        Buffer* dst,
        DeviceOffset dst_offset,
        const Buffer* src,
        DeviceOffset src_offset,
        DeviceSize size
    );

    /**
     * \brief Copy a texture region.
     *
     * \param dst Destination texture.
     * \param dst_subresource Destination subresource index.
     * \param dst_offset Destination offset in texels.
     * \param src Source texture.
     * \param src_subresource Source subresource index.
     * \param src_offset Source offset in texels.
     * \param extent Size in texels (-1 for maximum possible size).
     */
    void copy_texture_region(
        Texture* dst,
        uint32_t dst_subresource,
        uint3 dst_offset,
        const Texture* src,
        uint32_t src_subresource,
        uint3 src_offset,
        uint3 extent = uint3(-1)
    );

    /**
     * \brief Copy a texture to a buffer.
     *
     * \param dst Destination buffer.
     * \param dst_offset Destination offset in bytes.
     * \param dst_size Destination size in bytes.
     * \param dst_row_stride Destination row stride in bytes.
     * \param src Source texture.
     * \param src_subresource Source subresource index.
     * \param src_offset Source offset in texels.
     * \param extent Extent in texels (-1 for maximum possible extent).
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
     * \brief Upload host memory to a buffer.
     *
     * \param buffer Buffer to write to.
     * \param offset Buffer offset in bytes.
     * \param size Number of bytes to write.
     * \param data Host memory to write.
     */
    void upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data);

    /**
     * \brief Upload host memory to a texture.
     *
     * \param texture Texture to write to.
     * \param subresource Subresource index.
     * \param subresource_data Subresource data.
     */
    void upload_texture_data(Texture* texture, uint32_t subresource, SubresourceData subresource_data);

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
     * \brief Blit a SRV to an RTV.
     *
     * Blits the full extent of the source texture to the destination texture.
     *
     * \param dst RTV of the destination texture.
     * \param src SRV of the source texture.
     * \param filter Filtering mode to use.
     */
    void blit(ResourceView* dst, ResourceView* src, TextureFilteringMode filter = TextureFilteringMode::linear);

    /**
     * \brief Blit a texture to another texture.
     *
     * Blits the full extent of the source texture to the destination texture.
     *
     * \param dst Destination texture.
     * \param src Source texture.
     * \param filter Filtering mode to use.
     */
    void blit(Texture* dst, Texture* src, TextureFilteringMode filter = TextureFilteringMode::linear);

    /**
     * \brief Start encoding compute commands.
     *
     * The returned \c ComputeCommandEncoder is used to bind compute pipelines and issue dispatches.
     * The encoding is ended when the \c ComputeCommandEncoder is destroyed.
     */
    ComputeCommandEncoder encode_compute_commands();

    /**
     * \brief Start encoding render commands.
     *
     * The returned \c RenderCommandEncoder is used to bind graphics pipelines and issue dispatches.
     * The encoding is ended when the \c RenderCommandEncoder is destroyed.
     */
    RenderCommandEncoder encode_render_commands(Framebuffer* framebuffer);

    /**
     * \brief Start encoding ray tracing commands.
     *
     * The returned \c RayTracingCommandEncoder is used to bind ray tracing pipelines and issue dispatches.
     * It also serves for building and managing acceleration structures.
     * The encoding is ended when the \c RayTracingCommandEncoder is destroyed.
     */
    RayTracingCommandEncoder encode_ray_tracing_commands();

    // ------------------------------------------------------------------------
    // Debug events
    // ------------------------------------------------------------------------

    /// Begin a debug event.
    void begin_debug_event(const char* name, float3 color);

    /// End a debug event.
    void end_debug_event();

    gfx::ICommandBuffer* gfx_command_buffer() const { return m_gfx_command_buffer; }

    std::string to_string() const override;

private:
    /// Called by command encoders when they are destroyed.
    void end_encoder();

    gfx::IResourceCommandEncoder* get_gfx_resource_command_encoder();
    void end_current_gfx_encoder();

    Slang::ComPtr<gfx::ITransientResourceHeap> m_gfx_transient_resource_heap;
    Slang::ComPtr<gfx::ICommandBuffer> m_gfx_command_buffer;

    bool m_open{false};
    bool m_encoder_open{false};

    enum class EncoderType {
        none,
        resource,
        compute,
        render,
        raytracing,
    };

    EncoderType m_active_gfx_encoder{EncoderType::none};
    Slang::ComPtr<gfx::ICommandEncoder> m_gfx_command_encoder;

    std::vector<ref<cuda::InteropBuffer>> m_cuda_interop_buffers;

    // TODO remove this
    friend class Device;
    friend class ComputeCommandEncoder;
    friend class RenderCommandEncoder;
    friend class RayTracingCommandEncoder;
};

} // namespace sgl
