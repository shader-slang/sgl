#pragma once

#include "kali/rhi/fwd.h"
#include "kali/rhi/resource.h"

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/core/object.h"
#include "kali/math/vector_types.h"

#include <slang-gfx.h>

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace kali {

class Window;

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

    /// If not null, the device will be created on the specified adapter.
    AdapterLUID* adapter_luid{nullptr};

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

    const DeviceDesc& get_desc() const { return m_desc; }

    const DeviceInfo& get_info() const { return m_info; }

    DeviceType get_type() const { return m_type; }

    ref<Swapchain> create_swapchain(const SwapchainDesc& desc, ref<Window> window);

    ref<Buffer> create_buffer(const BufferDesc& desc, const void* init_data = nullptr);

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

    ref<Texture> create_texture(const TextureDesc& desc, const void* init_data = nullptr);

    ref<Sampler> create_sampler(const SamplerDesc& desc);

    ref<Program> create_program(const ProgramDesc& desc);

    // ref<Program> create_program(std::filesystem::path path, std::string entrypoint);

    ProgramManager& get_program_manager();

    gfx::IDevice* get_gfx_device() const { return m_gfx_device.get(); }
    gfx::ICommandQueue* get_gfx_queue() const { return m_gfx_queue.get(); }

    /// Enumerates all available adapters of a given device type.
    static std::vector<AdapterInfo> enumerate_adapters(DeviceType type = DeviceType::automatic);

private:
    DeviceDesc m_desc;
    DeviceInfo m_info;
    DeviceType m_type;
    Slang::ComPtr<gfx::IDevice> m_gfx_device;
    Slang::ComPtr<gfx::ICommandQueue> m_gfx_queue;
    Slang::ComPtr<slang::IGlobalSession> m_slang_session;

    ref<ProgramManager> m_program_manager;
};

} // namespace kali
