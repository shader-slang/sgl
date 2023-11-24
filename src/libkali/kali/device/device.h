#pragma once

#include "kali/device/fwd.h"
#include "kali/device/types.h"
#include "kali/device/native_handle.h"
#include "kali/device/resource.h"
#include "kali/device/raytracing.h"

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/core/object.h"
#include "kali/math/vector_types.h"

#include <slang-gfx.h>

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace kali {

class Window;

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
            "   name=\"{}\",\n"
            "   vendor_id=0x{:x},\n"
            "   device_id=0x{:x},\n"
            "   luid={}\n"
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

KALI_ENUM_INFO(
    DeviceType,
    {
        {DeviceType::automatic, "automatic"},
        {DeviceType::d3d12, "d3d12"},
        {DeviceType::vulkan, "vulkan"},
        {DeviceType::cpu, "cpu"},
        {DeviceType::cuda, "cuda"},
    }
);
KALI_ENUM_REGISTER(DeviceType);

struct DeviceDesc {
    DeviceType type{DeviceType::automatic};
    bool enable_debug_layers{false};

    /// Adapter LUID to select adapeter on which the device will be created.
    std::optional<AdapterLUID> adapter_luid;

    /// Default shader model to use when compiling shaders.
    ShaderModel default_shader_model = ShaderModel::sm_6_6;

    /// Path to the shader cache directory. Leave empty to disable shader cache.
    std::filesystem::path shader_cache_path;
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
    /// Limits of the device.
    DeviceLimits limits;
};

class KALI_API Device : public Object {
    KALI_OBJECT(Device)
public:
    Device(const DeviceDesc& desc = DeviceDesc{});
    ~Device();

    static ref<Device> create(const DeviceDesc& desc = DeviceDesc{}) { return make_ref<Device>(desc); }

    const DeviceDesc& desc() const { return m_desc; }

    DeviceType type() const { return m_desc.type; }

    const DeviceInfo& info() const { return m_info; }

    ShaderModel supported_shader_model() const { return m_supported_shader_model; }
    ShaderModel default_shader_model() const { return m_default_shader_model; }

    ref<Swapchain> create_swapchain(SwapchainDesc desc, ref<Window> window);

    ref<Buffer> create_buffer(BufferDesc desc, const void* init_data = nullptr);

    ref<Buffer> create_raw_buffer(
        size_t size,
        ResourceUsage usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
        MemoryType memory_type = MemoryType::device_local,
        const void* init_data = nullptr
    );

    ref<Buffer> create_typed_buffer(
        Format format,
        size_t element_count,
        ResourceUsage usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
        MemoryType memory_type = MemoryType::device_local,
        const void* init_data = nullptr
    );

    template<typename T>
    ref<Buffer> create_typed_buffer(
        size_t element_count,
        ResourceUsage usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
        MemoryType memory_type = MemoryType::device_local,
        const void* init_data = nullptr
    )
    {
        constexpr Format format = host_type_to_format<T>();
        static_assert(format != Format::unknown, "Unsupported type");
        return create_typed_buffer(format, element_count, usage, memory_type, init_data);
    }

    ref<Buffer> create_structured_buffer(
        size_t struct_size,
        size_t element_count,
        ResourceUsage usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
        MemoryType memory_type = MemoryType::device_local,
        const void* init_data = nullptr
    );

    template<typename T>
    ref<Buffer> create_structured_buffer(
        size_t element_count,
        ResourceUsage usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
        MemoryType memory_type = MemoryType::device_local,
        const T* init_data = nullptr
    )
    {
        return create_structured_buffer(sizeof(T), element_count, usage, memory_type, init_data);
    }

    ref<Texture> create_texture(TextureDesc desc, const void* init_data = nullptr);

    ref<Texture> create_texture_from_resource(TextureDesc desc, gfx::ITextureResource* resource);

    ref<Sampler> create_sampler(SamplerDesc desc);

    ref<Fence> create_fence(FenceDesc desc);

    ref<QueryPool> create_query_pool(QueryPoolDesc desc);

    ref<AccelerationStructure> create_acceleration_structure(AccelerationStructure::Desc desc);

    ref<SlangSession> create_slang_session(SlangSessionDesc desc);

    ref<SlangModule> load_module(const std::filesystem::path& path);
    ref<SlangModule> load_module_from_source(const std::string& source);

    ref<ComputePipelineState> create_compute_pipeline_state(ComputePipelineStateDesc desc);

    ref<GraphicsPipelineState> create_graphics_pipeline_state(GraphicsPipelineStateDesc desc);

    ref<CommandQueue> create_command_queue(CommandQueueDesc desc);

    ref<CommandStream> create_command_stream(CommandStreamDesc desc);

    CommandStream* command_stream() const { return m_command_stream; }

    void wait();

    void read_buffer(const Buffer* buffer, size_t offset, size_t size, void* out_data);

    template<typename T>
    std::vector<T> read_buffer(const Buffer* buffer, size_t index, size_t count)
    {
        std::vector<T> data(count);
        read_buffer(buffer, index * sizeof(T), count * sizeof(T), data.data());
        return data;
    }


    gfx::IDevice* get_gfx_device() const { return m_gfx_device; }
    slang::IGlobalSession* get_global_session() const { return m_global_session; }

    /// Returns the native API handle:
    /// - D3D12: ID3D12Device* (0)
    /// - Vulkan: VkInstance (0), VkPhysicalDevice (1), VkDevice (2)
    NativeHandle get_native_handle(uint32_t index = 0) const;

    /// Enumerates all available adapters of a given device type.
    static std::vector<AdapterInfo> enumerate_adapters(DeviceType type = DeviceType::automatic);

    /// Report live objects in the slang/gfx layer.
    /// This is useful for checking clean shutdown with all resources released properly.
    static void report_live_objects();

    /**
     * Try to enable D3D12 Agility SDK at runtime.
     * Note: This must be called before creating a device to have any effect.
     *
     * Prefer adding KALI_EXPORT_AGILITY_SDK to the main translation unit of executables
     * to tag the application binary to load the D3D12 Agility SDK.
     *
     * When using kali through as a Python extension tagging the main application
     * (Python interpreter) is not possible. The alternative is to use the
     * D3D12SDKConfiguration API introduced in Windows SDK 20348. This however
     * requires "Developer Mode" to be enabled and the executed Python interpreter to be
     * stored on the same drive as the kali library.
     *
     * @return Return true if D3D12 Agility SDK was successfully enabled.
     */
    static bool enable_agility_sdk();

    std::string to_string() const override;

private:
    DeviceDesc m_desc;
    DeviceInfo m_info;
    ShaderModel m_supported_shader_model{ShaderModel::unknown};
    ShaderModel m_default_shader_model{ShaderModel::unknown};
    Slang::ComPtr<gfx::IDevice> m_gfx_device;
    Slang::ComPtr<slang::IGlobalSession> m_global_session;

    ref<CommandStream> m_command_stream;
};

} // namespace kali
