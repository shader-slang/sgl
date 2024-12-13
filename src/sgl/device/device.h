// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/native_handle.h"
#include "sgl/device/resource.h"
#include "sgl/device/shader.h"

#include "sgl/core/fwd.h"
#include "sgl/core/config.h"
#include "sgl/core/macros.h"
#include "sgl/core/enum.h"
#include "sgl/core/object.h"
#include "sgl/core/platform.h"
#include "sgl/math/vector_types.h"

#include <slang-gfx.h>

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <queue>

namespace sgl {

class DebugPrinter;

/// Adapter LUID (locally unique identifier).
using AdapterLUID = std::array<uint8_t, 16>;

struct AdapterInfo {
    /// Descriptive name of the adapter.
    std::string name;

    /// Unique identifier for the vendor (only available for D3D12 and Vulkan).
    uint32_t vendor_id;

    /// Unique identifier for the physical device among devices from the vendor (only available for D3D12 and Vulkan).
    uint32_t device_id;

    /// Logically unique identifier of the adapter.
    AdapterLUID luid;

    std::string to_string() const
    {
        return fmt::format(
            "AdapterInfo(\n"
            "  name = \"{}\",\n"
            "  vendor_id = 0x{:x},\n"
            "  device_id = 0x{:x},\n"
            "  luid = {}\n"
            ")",
            name,
            vendor_id,
            device_id,
            luid
        );
    }
};

enum class DeviceType {
    automatic,
    d3d12,
    vulkan,
    cpu,
    cuda,
};

SGL_ENUM_INFO(
    DeviceType,
    {
        {DeviceType::automatic, "automatic"},
        {DeviceType::d3d12, "d3d12"},
        {DeviceType::vulkan, "vulkan"},
        {DeviceType::cpu, "cpu"},
        {DeviceType::cuda, "cuda"},
    }
);
SGL_ENUM_REGISTER(DeviceType);

struct DeviceDesc {
    /// The type of the device.
    DeviceType type{DeviceType::automatic};

    /// Enable debug layers.
    bool enable_debug_layers{false};

    /// Enable CUDA interoperability.
    bool enable_cuda_interop{false};

    /// Enable device side printing (adds performance overhead).
    bool enable_print{false};

    /// Enable automatic shader reload in response to file changes.
    /// Note: Currently windows and linux only.
    bool enable_hot_reload{true};

    /// Adapter LUID to select adapter on which the device will be created.
    std::optional<AdapterLUID> adapter_luid;

    /// Compiler options (used for default slang session).
    SlangCompilerOptions compiler_options;

    /// Path to the shader cache directory (optional).
    /// If a relative path is used, the cache is stored in the application data directory.
    std::optional<std::filesystem::path> shader_cache_path;
};

struct DeviceLimits {
    /// Maximum dimension for 1D textures.
    uint32_t max_texture_dimension_1d;
    /// Maximum dimensions for 2D textures.
    uint32_t max_texture_dimension_2d;
    /// Maximum dimensions for 3D textures.
    uint32_t max_texture_dimension_3d;
    /// Maximum dimensions for cube textures.
    uint32_t max_texture_dimension_cube;
    /// Maximum number of texture layers.
    uint32_t max_texture_array_layers;

    /// Maximum number of vertex input elements in a graphics pipeline.
    uint32_t max_vertex_input_elements;
    /// Maximum offset of a vertex input element in the vertex stream.
    uint32_t max_vertex_input_element_offset;
    /// Maximum number of vertex streams in a graphics pipeline.
    uint32_t max_vertex_streams;
    /// Maximum stride of a vertex stream.
    uint32_t max_vertex_stream_stride;

    /// Maximum number of threads per thread group.
    uint32_t max_compute_threads_per_group;
    /// Maximum dimensions of a thread group.
    uint3 max_compute_thread_group_size;
    /// Maximum number of thread groups per dimension in a single dispatch.
    uint3 max_compute_dispatch_thread_groups;

    /// Maximum number of viewports per pipeline.
    uint32_t max_viewports;
    /// Maximum viewport dimensions.
    uint2 max_viewport_dimensions;
    /// Maximum framebuffer dimensions.
    uint3 max_framebuffer_dimensions;

    /// Maximum samplers visible in a shader stage.
    uint32_t max_shader_visible_samplers;
};

struct DeviceInfo {
    /// The type of the device.
    DeviceType type;
    /// The name of the graphics API being used by this device.
    std::string api_name;
    /// The name of the graphics adapter.
    std::string adapter_name;
    /// The logically unique identifier of the graphics adapter.
    AdapterLUID adapter_luid;
    /// The frequency of the timestamp counter.
    /// To resolve a timestamp to seconds, divide by this value.
    uint64_t timestamp_frequency;
    /// Limits of the device.
    DeviceLimits limits;
};

struct ShaderCacheStats {
    /// Number of entries in the cache.
    size_t entry_count;
    /// Number of hits in the cache.
    size_t hit_count;
    /// Number of misses in the cache.
    size_t miss_count;
};

/// Event data for hot reload hook.
struct ShaderHotReloadEvent { };
using ShaderHotReloadCallback = std::function<void(const ShaderHotReloadEvent&)>;


using DeviceCloseCallback = std::function<void(Device*)>;

class SGL_API Device : public Object {
    SGL_OBJECT(Device)
public:
    Device(const DeviceDesc& desc = DeviceDesc{});
    ~Device();

    static ref<Device> create(const DeviceDesc& desc = DeviceDesc{}) { return make_ref<Device>(desc); }

    const DeviceDesc& desc() const { return m_desc; }

    /// Type of the graphics API used by this device.
    DeviceType type() const { return m_desc.type; }

    /// Device information.
    const DeviceInfo& info() const { return m_info; }

    /// Shader cache statistics.
    ShaderCacheStats shader_cache_stats() const;

    /// The highest shader model supported by the device.
    ShaderModel supported_shader_model() const { return m_supported_shader_model; }

    /// List of features supported by the device.
    const std::vector<std::string>& features() const { return m_features; }

    /// True if the device supports CUDA interoperability.
    bool supports_cuda_interop() const { return m_supports_cuda_interop; }

    /// Returns the supported resource states for a given format.
    ResourceStateSet get_format_supported_resource_states(Format format) const;

    /// Default slang session.
    SlangSession* slang_session() const { return m_slang_session; }

    /**
     * \brief Close the device.
     *
     * This function should be called before the device is released.
     * It waits for all pending work to be completed and releases internal
     * resources, removing all cyclic references that might prevent the device
     * from being destroyed. After closing the device, no new resources must be
     * created and no new work must be submitted.
     *
     * \note The Python extension will automatically close all open devices
     * when the interpreter is terminated through an `atexit` handler. If a
     * device is to be destroyed at runtime, it must be closed explicitly.
     */
    void close();

    /// Close all open devices.
    static void close_all_devices();

    // Resource creation

    /**
     * \brief Create a new swapchain.
     *
     * \param format Format of the swapchain images.
     * \param width Width of the swapchain images in pixels.
     * \param height Height of the swapchain images in pixels.
     * \param image_count Number of swapchain images.
     * \param enable_vsync Enable/disable vertical synchronization.
     * \param window Window to create the swapchain for.
     * \return New swapchain object.
     */
    ref<Swapchain> create_swapchain(SwapchainDesc desc, Window* window);

    /**
     * \brief Create a new swapchain.
     *
     * \param format Format of the swapchain images.
     * \param width Width of the swapchain images in pixels.
     * \param height Height of the swapchain images in pixels.
     * \param image_count Number of swapchain images.
     * \param enable_vsync Enable/disable vertical synchronization.
     * \param window_handle Native window handle to create the swapchain for.
     * \return New swapchain object.
     */
    ref<Swapchain> create_swapchain(SwapchainDesc desc, WindowHandle window_handle);

    /**
     * \brief Create a new buffer.
     *
     * \param size Buffer size in bytes.
     * \param element_count Buffer size in number of struct elements. Can be used instead of \c size.
     * \param struct_size Struct size in bytes.
     * \param struct_type Struct type. Can be used instead of \c struct_size to specify the size of the struct.
     * \param format Buffer format. Used when creating typed buffer views.
     * \param initial_state Initial resource state.
     * \param usage Resource usage flags.
     * \param memory_type Memory type.
     * \param debug_name Resource debug name.
     * \param data Initial data to upload to the buffer.
     * \param data_size Size of the initial data in bytes.
     * \return New buffer object.
     */
    ref<Buffer> create_buffer(BufferDesc desc);

    /**
     * \brief Create a new texture.
     *
     * \param type Resource type (optional). Type is inferred from width, height, depth if not specified.
     * \param format Texture format.
     * \param width Width in pixels.
     * \param height Height in pixels.
     * \param depth Depth in pixels.
     * \param array_size Number of array slices (1 for non-array textures).
     * \param mip_count Number of mip levels (0 for auto-generated mips).
     * \param sample_count Number of samples per pixel (1 for non-multisampled textures).
     * \param quality Quality level for multisampled textures.
     * \param usage Resource usage.
     * \param memory_type Memory type.
     * \param debug_name Debug name.
     * \param data Initial data.
     * \return New texture object.
     */
    ref<Texture> create_texture(TextureDesc desc);

    ref<Texture> create_texture_from_resource(TextureDesc desc, gfx::ITextureResource* resource, bool deferred_release);

    /**
     * \brief Create a new sampler.
     *
     * \param min_filter Minification filter.
     * \param mag_filter Magnification filter.
     * \param mip_filter Mip-map filter.
     * \param reduction_op Reduction operation.
     * \param address_u Texture addressing mode for the U coordinate.
     * \param address_v Texture addressing mode for the V coordinate.
     * \param address_w Texture addressing mode for the W coordinate.
     * \param mip_lod_bias Mip-map LOD bias.
     * \param max_anisotropy Maximum anisotropy.
     * \param comparison_func Comparison function.
     * \param border_color Border color.
     * \param min_lod Minimum LOD level.
     * \param max_lod Maximum LOD level.
     * \return New sampler object.
     */
    ref<Sampler> create_sampler(SamplerDesc desc);

    /**
     * \brief Create a new fence.
     *
     * \param initial_value Initial fence value.
     * \param shared Create a shared fence.
     * \return New fence object.
     */
    ref<Fence> create_fence(FenceDesc desc);

    /**
     * \brief Create a new query pool.
     *
     * \param type Query type.
     * \param count Number of queries in the pool.
     * \return New query pool object.
     */
    ref<QueryPool> create_query_pool(QueryPoolDesc desc);

    /**
     * \brief Create a new input layout.
     *
     * \param input_elements List of input elements (see \ref InputElementDesc for details).
     * \param vertex_streams List of vertex streams (see \ref VertexStreamDesc for details).
     * \return New input layout object.
     */
    ref<InputLayout> create_input_layout(InputLayoutDesc desc);

    /**
     * \brief Create a new framebuffer.
     *
     * \param render_target List of render targets (see \ref FramebufferAttachmentDesc for details).
     * \param depth_stencil Optional depth-stencil attachment (see \ref FramebufferAttachmentDesc for details).
     * \return New framebuffer object.
     */
    ref<Framebuffer> create_framebuffer(FramebufferDesc desc);

    AccelerationStructurePrebuildInfo
    get_acceleration_structure_prebuild_info(const AccelerationStructureBuildInputs& build_inputs);

    ref<AccelerationStructure> create_acceleration_structure(AccelerationStructureDesc desc);

    ref<ShaderTable> create_shader_table(ShaderTableDesc desc);

    /**
     * \brief Create a new slang session.
     *
     * \param compiler_options Compiler options (see \ref SlangCompilerOptions for details).
     * \return New slang session object.
     */
    ref<SlangSession> create_slang_session(SlangSessionDesc desc);

    ref<SlangModule> load_module(std::string_view module_name);

    ref<SlangModule> load_module_from_source(
        std::string_view module_name,
        std::string_view source,
        std::optional<std::filesystem::path> path = {}
    );

    ref<ShaderProgram> link_program(
        std::vector<ref<SlangModule>> modules,
        std::vector<ref<SlangEntryPoint>> entry_points,
        std::optional<SlangLinkOptions> link_options = {}
    );

    ref<ShaderProgram> load_program(
        std::string_view module_name,
        std::vector<std::string_view> entry_point_names,
        std::optional<std::string_view> additional_source = {},
        std::optional<SlangLinkOptions> link_options = {}
    );

    void reload_all_programs();

    ref<MutableShaderObject> create_mutable_shader_object(const ShaderProgram* shader_program);

    ref<MutableShaderObject> create_mutable_shader_object(const TypeLayoutReflection* type_layout);

    ref<MutableShaderObject> create_mutable_shader_object(ReflectionCursor cursor);

    ref<ComputePipeline> create_compute_pipeline(ComputePipelineDesc desc);

    ref<GraphicsPipeline> create_graphics_pipeline(GraphicsPipelineDesc desc);

    ref<RayTracingPipeline> create_ray_tracing_pipeline(RayTracingPipelineDesc desc);

    ref<ComputeKernel> create_compute_kernel(ComputeKernelDesc desc);

    ref<CommandBuffer> create_command_buffer();

    void _set_open_command_buffer(CommandBuffer* command_buffer);
    Slang::ComPtr<gfx::ITransientResourceHeap> _get_or_create_transient_resource_heap();

    CommandBuffer* _begin_shared_command_buffer();
    void _end_shared_command_buffer(bool wait);

    /**
     * \brief Submit a command buffer to the device.
     *
     * The returned submission ID can be used to wait for the command buffer to complete.
     *
     * \param command_buffer Command buffer to submit.
     * \param queue Command queue to submit to.
     * \return Submission ID.
     */
    uint64_t submit_command_buffer(CommandBuffer* command_buffer, CommandQueueType queue = CommandQueueType::graphics);

    /**
     * \brief Check if a command buffer is complete.
     *
     * \param id Submission ID.
     * \return True if the command buffer is complete.
     */
    bool is_command_buffer_complete(uint64_t id);

    /**
     * \brief Wait for a command buffer to complete.
     *
     * \param id Submission ID.
     */
    void wait_command_buffer(uint64_t id);

    /**
     * \brief Wait for the command queue to be idle.
     *
     * \param queue Command queue to wait for.
     */
    void wait_for_idle(CommandQueueType queue = CommandQueueType::graphics);

    /**
     * \brief Synchronize CUDA -> device.
     *
     * This signals a shared CUDA semaphore from the CUDA stream and then waits for the signal on the command queue.
     *
     * \param cuda_stream CUDA stream
     */
    void sync_to_cuda(void* cuda_stream = 0);

    /**
     * \brief Synchronize device -> CUDA.
     *
     * This waits for a shared CUDA semaphore on the CUDA stream, making sure all commands on the device have completed.
     *
     * \param cuda_stream CUDA stream
     */
    void sync_to_device(void* cuda_stream = 0);

    /**
     * \brief Execute garbage collection.
     *
     * This function should be called regularly to execute deferred releases (at least once a frame).
     */
    void run_garbage_collection();

    ref<MemoryHeap> create_memory_heap(MemoryHeapDesc desc);

    MemoryHeap* upload_heap() const { return m_upload_heap; }
    MemoryHeap* read_back_heap() const { return m_read_back_heap; }

    DebugPrinter* debug_printer() const { return m_debug_printer.get(); }

    /// Block and flush all shader side debug print output.
    void flush_print();

    /// Block and flush all shader side debug print output to a string.
    std::string flush_print_to_string();

    /// Wait for all device work to complete.
    void wait();

    /**
     * Upload host memory to buffer.
     *
     * \param buffer Buffer to write to.
     * \param data Data to write.
     * \param size Size of the data in bytes.
     * \param offset Offset in the buffer to write to.
     */
    void upload_buffer_data(Buffer* buffer, const void* data, size_t size, size_t offset = 0);

    /**
     * Read buffer data to host memory.
     * \note This will wait until the data is copied back to host memory.
     *
     * \param buffer Buffer to read from.
     * \param data Data to buffer to read to.
     * \param size Size of the data in bytes.
     * \param offset Offset in the buffer to read from.
     */
    void read_buffer_data(const Buffer* buffer, void* data, size_t size, size_t offset = 0);

    /**
     * Upload host memory to texture.
     *
     * \param texture Texture to write to.
     * \param subresource Subresource index.
     * \param subresource_data Subresource data.
     */
    void upload_texture_data(Texture* texture, uint32_t subresource, SubresourceData subresource_data);

    /**
     * Read texture data to host memory.
     * \note This will wait until the data is copied back to host memory.
     *
     * \param texture Texture to read from.
     * \param subresource Subresource index.
     * \return Subresource data in host memory.
     */
    OwnedSubresourceData read_texture_data(const Texture* texture, uint32_t subresource);

    void deferred_release(ISlangUnknown* object);

    gfx::IDevice* gfx_device() const { return m_gfx_device; }
    gfx::ICommandQueue* gfx_graphics_queue() const { return m_gfx_graphics_queue; }

    slang::IGlobalSession* global_session() const { return m_global_session; }

    /// Returns the native API handle:
    /// - D3D12: ID3D12Device* (0)
    /// - Vulkan: VkInstance (0), VkPhysicalDevice (1), VkDevice (2)
    NativeHandle get_native_handle(uint32_t index = 0) const;

    /// Returns the native API handle for the command queue:
    /// - D3D12: ID3D12CommandQueue*
    /// - Vulkan: VkQueue (Vulkan)
    NativeHandle get_native_command_queue_handle(CommandQueueType queue = CommandQueueType::graphics) const;


    /// Enumerates all available adapters of a given device type.
    static std::vector<AdapterInfo> enumerate_adapters(DeviceType type = DeviceType::automatic);

    /// Report live objects in the slang/gfx layer.
    /// This is useful for checking clean shutdown with all resources released properly.
    static void report_live_objects();

    /**
     * Try to enable D3D12 Agility SDK at runtime.
     * Note: This must be called before creating a device to have any effect.
     *
     * Prefer adding SGL_EXPORT_AGILITY_SDK to the main translation unit of executables
     * to tag the application binary to load the D3D12 Agility SDK.
     *
     * When using sgl through as a Python extension tagging the main application
     * (Python interpreter) is not possible. The alternative is to use the
     * D3D12SDKConfiguration API introduced in Windows SDK 20348. This however
     * requires "Developer Mode" to be enabled and the executed Python interpreter to be
     * stored on the same drive as the sgl library.
     *
     * \return Return true if D3D12 Agility SDK was successfully enabled.
     */
    static bool enable_agility_sdk();

    /// Register a hot reload hook, called immediately after any module is reloaded.
    void register_shader_hot_reload_callback(ShaderHotReloadCallback call_back)
    {
        m_shader_hot_reload_callbacks.push_back(call_back);
    }

    /// Register a device close callback, called at start of device close.
    void register_device_close_callback(DeviceCloseCallback call_back)
    {
        m_device_close_callbacks.push_back(call_back);
    }

    cuda::Device* cuda_device() const { return m_cuda_device.get(); }

    std::string to_string() const override;

    Blitter* _blitter();
    HotReload* _hot_reload() { return m_hot_reload; }

    /// Called by hot reload system after reload occurs, to trigger the hooks.
    void _on_hot_reload()
    {
        for (auto& hook : m_shader_hot_reload_callbacks)
            hook({});
    }

private:
    DeviceDesc m_desc;
    DeviceInfo m_info;
    ShaderModel m_supported_shader_model{ShaderModel::unknown};

    bool m_closed{false};

    bool m_shader_cache_enabled{false};
    std::filesystem::path m_shader_cache_path;

    Slang::ComPtr<gfx::IDevice> m_gfx_device;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_graphics_queue;
    Slang::ComPtr<slang::IGlobalSession> m_global_session;

    ref<SlangSession> m_slang_session;

    std::vector<std::string> m_features;

    ref<Fence> m_global_fence;

    ref<MemoryHeap> m_upload_heap;
    ref<MemoryHeap> m_read_back_heap;

    std::unique_ptr<DebugPrinter> m_debug_printer;

    /// Currently open command buffer.
    /// Due to limitations in gfx, only one command buffer can be open at a time.
    CommandBuffer* m_open_command_buffer{nullptr};
    ref<CommandBuffer> m_shared_command_buffer;

    /// Currently active transient resource heap.
    /// All command buffers are created on this heap.
    Slang::ComPtr<gfx::ITransientResourceHeap> m_current_transient_resource_heap;

    /// Transient resource heaps available for reuse.
    std::queue<Slang::ComPtr<gfx::ITransientResourceHeap>> m_transient_resource_heap_pool;

    /// Transient resource heaps that are currently in flight.
    std::queue<std::pair<Slang::ComPtr<gfx::ITransientResourceHeap>, uint64_t>> m_in_flight_transient_resource_heaps;

    /// List of callbacks for hot reload event
    std::vector<ShaderHotReloadCallback> m_shader_hot_reload_callbacks;

    /// List of callbacks for shutdown event
    std::vector<DeviceCloseCallback> m_device_close_callbacks;

    struct DeferredRelease {
        uint64_t fence_value;
        Slang::ComPtr<ISlangUnknown> object;
    };

    std::queue<DeferredRelease> m_deferred_release_queue;

#if SGL_HAS_NVAPI
    class PipelineCreationAPIDispatcher;
    std::unique_ptr<PipelineCreationAPIDispatcher> m_api_dispatcher;
#endif

    ref<Blitter> m_blitter;
    ref<HotReload> m_hot_reload;

    bool m_supports_cuda_interop{false};
    ref<cuda::Device> m_cuda_device;
    ref<cuda::ExternalSemaphore> m_cuda_semaphore;
};

} // namespace sgl
