#include "sampler.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"

#include "kali/core/error.h"

#include "types.inl"

namespace kali {

Sampler::Sampler(SamplerDesc desc, ref<Device> device)
    : m_desc(std::move(desc))
    , m_device(std::move(device))
{
    gfx::ISamplerState::Desc gfx_desc{
        .minFilter = get_gfx_texture_filtering_mode(m_desc.min_filter),
        .magFilter = get_gfx_texture_filtering_mode(m_desc.mag_filter),
        .mipFilter = get_gfx_texture_filtering_mode(m_desc.mip_filter),
        .reductionOp = get_gfx_texture_reduction_op(m_desc.reduction_op),
        .addressU = get_gfx_texture_addressing_mode(m_desc.address_u),
        .addressV = get_gfx_texture_addressing_mode(m_desc.address_v),
        .addressW = get_gfx_texture_addressing_mode(m_desc.address_w),
        .mipLODBias = m_desc.mip_lod_bias,
        .maxAnisotropy = m_desc.max_anisotropy,
        .comparisonFunc = get_gfx_comparison_func(m_desc.comparison_func),
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
    if (m_device->get_type() == DeviceType::d3d12)
        return NativeHandle(D3D12_CPU_DESCRIPTOR_HANDLE{handle.handleValue});
#endif
#if KALI_HAS_VULKAN
    if (m_device->get_type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkSampler>(handle.handleValue));
#endif
    return {};
}

} // namespace kali