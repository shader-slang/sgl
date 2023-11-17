#include "sampler.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"

#include "kali/core/config.h"
#include "kali/core/error.h"
#include "kali/core/string.h"

namespace kali {

Sampler::Sampler(ref<Device> device, SamplerDesc desc)
    : m_device(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::ISamplerState::Desc gfx_desc{
        .minFilter = static_cast<gfx::TextureFilteringMode>(m_desc.min_filter),
        .magFilter = static_cast<gfx::TextureFilteringMode>(m_desc.mag_filter),
        .mipFilter = static_cast<gfx::TextureFilteringMode>(m_desc.mip_filter),
        .reductionOp = static_cast<gfx::TextureReductionOp>(m_desc.reduction_op),
        .addressU = static_cast<gfx::TextureAddressingMode>(m_desc.address_u),
        .addressV = static_cast<gfx::TextureAddressingMode>(m_desc.address_v),
        .addressW = static_cast<gfx::TextureAddressingMode>(m_desc.address_w),
        .mipLODBias = m_desc.mip_lod_bias,
        .maxAnisotropy = m_desc.max_anisotropy,
        .comparisonFunc = static_cast<gfx::ComparisonFunc>(m_desc.comparison_func),
        .borderColor = {m_desc.border_color.x, m_desc.border_color.y, m_desc.border_color.z, m_desc.border_color.w},
        .minLOD = m_desc.min_lod,
        .maxLOD = m_desc.max_lod,
    };
    SLANG_CALL(m_device->get_gfx_device()->createSamplerState(gfx_desc, m_gfx_sampler_state.writeRef()));
}

NativeHandle Sampler::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_sampler_state->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->type() == DeviceType::d3d12)
        return NativeHandle(D3D12_CPU_DESCRIPTOR_HANDLE{handle.handleValue});
#endif
#if KALI_HAS_VULKAN
    if (m_device->type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkSampler>(handle.handleValue));
#endif
    return {};
}

std::string Sampler::to_string() const
{
    return fmt::format(
        "Sampler(\n"
        "   device={},\n"
        "   desc={}\n"
        ")",
        m_device,
        string::indent(m_desc.to_string())
    );
}

} // namespace kali
