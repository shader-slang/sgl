// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/fence.h"
#include "sgl/device/resource.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/raytracing.h"

#include "sgl/core/object.h"
#include "sgl/core/static_vector.h"
#include "sgl/math/vector_types.h"

#include <span>

namespace sgl {

struct RenderState {
    uint32_t stencil_ref{0};
    static_vector<Viewport, 16> viewports;
    static_vector<ScissorRect, 16> scissor_rects;
    static_vector<BufferWithOffset, 16> vertex_buffers;
    BufferWithOffset index_buffer;
    IndexFormat index_format{IndexFormat::uint32};
};

struct RenderPassColorAttachment {
    TextureView* view{nullptr};
    TextureView* resolve_target{nullptr};
    LoadOp load_op{LoadOp::dont_care};
    StoreOp store_op{StoreOp::store};
    float4 clear_value{0.f};
};

struct RenderPassDepthStencilAttachment {
    TextureView* view{nullptr};
    LoadOp depth_load_op{LoadOp::dont_care};
    StoreOp depth_store_op{StoreOp::store};
    float depth_clear_value{1.f};
    bool depth_read_only{false};
    LoadOp stencil_load_op{LoadOp::dont_care};
    StoreOp stencil_store_op{StoreOp::dont_care};
    uint8_t stencil_clear_value{0};
    bool stencil_read_only{false};
};

struct RenderPassDesc {
    std::vector<RenderPassColorAttachment> color_attachments;
    std::optional<RenderPassDepthStencilAttachment> depth_stencil_attachment;
};

struct DrawArguments {
    uint32_t vertex_count{0};
    uint32_t instance_count{1};
    uint32_t start_vertex_location{0};
    uint32_t start_instance_location{0};
    uint32_t start_index_location{0};
};

class SGL_API PassEncoder : public Object {
    SGL_OBJECT(PassEncoder)
public:
    /// Push a debug group.
    void push_debug_group(const char* name, float3 color);

    /// Pop a debug group.
    void pop_debug_group();

    /**
     * Insert a debug marker.
     * \param name Name of the marker.
     * \param color Color of the marker.
     */
    void insert_debug_marker(const char* name, float3 color);

    virtual void end();

protected:
    rhi::IPassEncoder* m_rhi_pass_encoder;

    friend class CommandEncoder;
};

class SGL_API RenderPassEncoder : public PassEncoder {
    SGL_OBJECT(RenderPassEncoder)
public:
    ShaderObject* bind_pipeline(RenderPipeline* pipeline);
    void bind_pipeline(RenderPipeline* pipeline, ShaderObject* root_object);

    void set_render_state(const RenderState& state);

    void draw(const DrawArguments& args);
    void draw_indexed(const DrawArguments& args);

    void draw_indirect(uint32_t max_draw_count, BufferWithOffset arg_buffer, BufferWithOffset count_buffer = {});

    void
    draw_indexed_indirect(uint32_t max_draw_count, BufferWithOffset arg_buffer, BufferWithOffset count_buffer = {});

    void draw_mesh_tasks(uint3 dimensions);

    void end() override;

private:
    rhi::IRenderPassEncoder* m_rhi_render_pass_encoder;

    friend class CommandEncoder;
};

class SGL_API ComputePassEncoder : public PassEncoder {
    SGL_OBJECT(ComputePassEncoder)
public:
    ShaderObject* bind_pipeline(ComputePipeline* pipeline);
    void bind_pipeline(ComputePipeline* pipeline, ShaderObject* root_object);

    void dispatch(uint3 thread_count);
    void dispatch_thread_groups(uint3 thread_group_count);
    void dispatch_thread_groups_indirect(const Buffer* cmd_buffer, DeviceOffset offset);

    // virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(uint32_t x, uint32_t y, uint32_t z) = 0;
    // virtual SLANG_NO_THROW void SLANG_MCALL dispatchComputeIndirect(IBuffer* argBuffer, uint64_t offset) = 0;

    void end() override;

private:
    rhi::IComputePassEncoder* m_rhi_compute_pass_encoder;

    friend class CommandEncoder;
};

class SGL_API RayTracingPassEncoder : public PassEncoder {
    SGL_OBJECT(RayTracingPassEncoder)
public:
    ShaderObject* bind_pipeline(RayTracingPipeline* pipeline, ShaderTable* shader_table);
    void bind_pipeline(RayTracingPipeline* pipeline, ShaderTable* shader_table, ShaderObject* root_object);

    void dispatch_rays(uint32_t ray_gen_shader_index, uint3 dimensions);

    void end() override;

private:
    rhi::IRayTracingPassEncoder* m_rhi_ray_tracing_pass_encoder;

    friend class CommandEncoder;
};

class SGL_API CommandEncoder : public DeviceResource {
    SGL_OBJECT(CommandEncoder)
public:
    CommandEncoder(ref<Device> device, Slang::ComPtr<rhi::ICommandEncoder> rhi_command_encoder);

    ref<RenderPassEncoder> begin_render_pass(const RenderPassDesc& desc);
    ref<ComputePassEncoder> begin_compute_pass();
    ref<RayTracingPassEncoder> begin_ray_tracing_pass();

    /**
     * \brief Copy a buffer region.
     *
     * \param dst Destination buffer.
     * \param dst_offset Destination offset in bytes.
     * \param src Source buffer.
     * \param src_offset Source offset in bytes.
     * \param size Size in bytes.
     */
    void copy_buffer(Buffer* dst, DeviceOffset dst_offset, const Buffer* src, DeviceOffset src_offset, DeviceSize size);

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
    void copy_texture(
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

    void clear_buffer(Buffer* buffer, BufferRange range = {});

    void clear_texture(
        Texture* texture,
        float4 clear_value,
        SubresourceRange range = {},
        bool clear_depth = true,
        bool clear_stencil = true
    );

    void clear_texture(
        Texture* texture,
        uint4 clear_value,
        SubresourceRange range = {},
        bool clear_depth = true,
        bool clear_stencil = true
    );

    /**
     * \brief Blit a texture view.
     *
     * Blits the full extent of the source texture to the destination texture.
     *
     * \param dst View of the destination texture.
     * \param src View of the source texture.
     * \param filter Filtering mode to use.
     */
    void blit(TextureView* dst, TextureView* src, TextureFilteringMode filter = TextureFilteringMode::linear);

    /**
     * \brief Blit a texture.
     *
     * Blits the full extent of the source texture to the destination texture.
     *
     * \param dst Destination texture.
     * \param src Source texture.
     * \param filter Filtering mode to use.
     */
    void blit(Texture* dst, Texture* src, TextureFilteringMode filter = TextureFilteringMode::linear);

    void resolve_query(QueryPool* query_pool, uint32_t index, uint32_t count, Buffer* buffer, DeviceOffset offset);

    void build_acceleration_structure(
        const AccelerationStructureBuildDesc& desc,
        AccelerationStructure* dst,
        AccelerationStructure* src,
        BufferWithOffset scratch_buffer,
        std::span<AccelerationStructureQueryDesc> queries = std::span<AccelerationStructureQueryDesc>()
    );

    void copy_acceleration_structure(
        AccelerationStructure* dst,
        AccelerationStructure* src,
        AccelerationStructureCopyMode mode
    );

    void query_acceleration_structure_properties(
        std::span<AccelerationStructure*> acceleration_structures,
        std::span<AccelerationStructureQueryDesc> queries
    );

    void serialize_acceleration_structure(BufferWithOffset dst, AccelerationStructure* src);
    void deserialize_acceleration_structure(AccelerationStructure* dst, BufferWithOffset src);

    /**
     * Transition resource state of a buffer and add a barrier if state has changed.
     * \param buffer Buffer
     * \param state New state
     */
    void set_buffer_state(Buffer* buffer, ResourceState state);

    /**
     * Transition resource state of a texture and add a barrier if state has changed.
     * \param texture Texture
     * \param state New state
     */
    void set_texture_state(Texture* texture, ResourceState state);

    /**
     * Transition resource state of texture sub-resources and add a barrier if state has changed.
     * \param texture Texture
     * \param range Subresource range
     * \param state New state
     */
    void set_texture_state(Texture* texture, SubresourceRange range, ResourceState state);

    /// Push a debug group.
    void push_debug_group(const char* name, float3 color);

    /// Pop a debug group.
    void pop_debug_group();

    /**
     * Insert a debug marker.
     * \param name Name of the marker.
     * \param color Color of the marker.
     */
    void insert_debug_marker(const char* name, float3 color);

    /**
     * \brief Write a timestamp.
     *
     * \param query_pool Query pool.
     * \param index Index of the query.
     */
    void write_timestamp(QueryPool* query_pool, uint32_t index);

    ref<CommandBuffer> finish();

    NativeHandle get_native_handle() const;

    rhi::ICommandEncoder* rhi_command_encoder() const { return m_rhi_command_encoder; }

    std::string to_string() const override;

private:
    Slang::ComPtr<rhi::ICommandEncoder> m_rhi_command_encoder;

    std::vector<ref<cuda::InteropBuffer>> m_cuda_interop_buffers;

    bool m_open{false};

    ref<RenderPassEncoder> m_render_pass_encoder;
    ref<ComputePassEncoder> m_compute_pass_encoder;
    ref<RayTracingPassEncoder> m_ray_tracing_pass_encoder;
};

#if 0

class SGL_API ComputeCommandEncoder {
public:
    ComputeCommandEncoder(ComputeCommandEncoder&& other) noexcept
        : m_command_buffer(std::exchange(other.m_command_buffer, nullptr))
        , m_rhi_compute_command_encoder(std::exchange(other.m_rhi_compute_command_encoder, nullptr))
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
    ComputeCommandEncoder(CommandBuffer* command_buffer, rhi::IComputeCommandEncoder* rhi_compute_command_encoder)
        : m_command_buffer(command_buffer)
        , m_rhi_compute_command_encoder(rhi_compute_command_encoder)
    {
    }

    ComputeCommandEncoder(const ComputeCommandEncoder&) = delete;
    ComputeCommandEncoder& operator=(const ComputeCommandEncoder&) = delete;
    ComputeCommandEncoder& operator=(ComputeCommandEncoder&& other) noexcept = delete;

    CommandBuffer* m_command_buffer;
    rhi::IComputeCommandEncoder* m_rhi_compute_command_encoder;
    const ComputePipeline* m_bound_pipeline{nullptr};
    ref<const ShaderObject> m_bound_shader_object;

    friend class CommandBuffer;
};

class SGL_API RenderCommandEncoder {
public:
    RenderCommandEncoder(RenderCommandEncoder&& other) noexcept
        : m_command_buffer(std::exchange(other.m_command_buffer, nullptr))
        , m_rhi_render_command_encoder(std::exchange(other.m_rhi_render_command_encoder, nullptr))
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
    //     rhi_count samples_per_pixel, rhi_count pixel_count, const sample_position* sample_positions) = 0;


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
    RenderCommandEncoder(CommandBuffer* command_buffer, rhi::IRenderCommandEncoder* rhi_render_command_encoder)
        : m_command_buffer(command_buffer)
        , m_rhi_render_command_encoder(rhi_render_command_encoder)
    {
    }

    RenderCommandEncoder(const RenderCommandEncoder&) = delete;
    RenderCommandEncoder& operator=(const RenderCommandEncoder&) = delete;
    RenderCommandEncoder& operator=(RenderCommandEncoder&& other) noexcept = delete;

    CommandBuffer* m_command_buffer;
    rhi::IRenderCommandEncoder* m_rhi_render_command_encoder;
    const GraphicsPipeline* m_bound_pipeline{nullptr};
    ref<const ShaderObject> m_bound_shader_object;

    friend class CommandBuffer;
};

class SGL_API RayTracingCommandEncoder {
public:
    RayTracingCommandEncoder(RayTracingCommandEncoder&& other) noexcept
        : m_command_buffer(std::exchange(other.m_command_buffer, nullptr))
        , m_rhi_ray_tracing_command_encoder(std::exchange(other.m_rhi_ray_tracing_command_encoder, nullptr))
    {
    }

    ~RayTracingCommandEncoder();

    void end();

    ref<TransientShaderObject> bind_pipeline(const RayTracingPipeline* pipeline);
    void bind_pipeline(const RayTracingPipeline* pipeline, const ShaderObject* shader_object);
    void dispatch_rays(uint32_t ray_gen_shader_index, const ShaderTable* shader_table, uint3 dimensions);


private:
    RayTracingCommandEncoder(
        CommandBuffer* command_buffer,
        rhi::IRayTracingCommandEncoder* rhi_ray_tracing_command_encoder
    )
        : m_command_buffer(command_buffer)
        , m_rhi_ray_tracing_command_encoder(rhi_ray_tracing_command_encoder)
    {
    }

    RayTracingCommandEncoder(const RayTracingCommandEncoder&) = delete;
    RayTracingCommandEncoder& operator=(const RayTracingCommandEncoder&) = delete;
    RayTracingCommandEncoder& operator=(RayTracingCommandEncoder&& other) noexcept = delete;

    CommandBuffer* m_command_buffer;
    rhi::IRayTracingCommandEncoder* m_rhi_ray_tracing_command_encoder;
    const RayTracingPipeline* m_bound_pipeline{nullptr};
    ref<const ShaderObject> m_bound_shader_object;

    friend class CommandBuffer;
};

#endif

class SGL_API CommandBuffer : public DeviceResource {
    SGL_OBJECT(CommandBuffer)
public:
    CommandBuffer(ref<Device> device);
    ~CommandBuffer();

    rhi::ICommandBuffer* rhi_command_buffer() const { return m_rhi_command_buffer; }

    std::string to_string() const override;

private:
    Slang::ComPtr<rhi::ICommandBuffer> m_rhi_command_buffer;

    std::vector<ref<cuda::InteropBuffer>> m_cuda_interop_buffers;

    friend class Device;
};

} // namespace sgl
