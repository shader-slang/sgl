#pragma once

#include "kali/rhi/fwd.h"
#include "kali/rhi/types.h"
#include "kali/rhi/native_handle.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/math/vector_types.h"

#include <slang-gfx.h>

namespace kali {

struct SamplerDesc {
    TextureFilteringMode min_filter{TextureFilteringMode::linear};
    TextureFilteringMode mag_filter{TextureFilteringMode::linear};
    TextureFilteringMode mip_filter{TextureFilteringMode::linear};
    TextureReductionOp reduction_op{TextureReductionOp::average};
    TextureAddressingMode address_u{TextureAddressingMode::wrap};
    TextureAddressingMode address_v{TextureAddressingMode::wrap};
    TextureAddressingMode address_w{TextureAddressingMode::wrap};
    float mip_lod_bias{0.f};
    uint32_t max_anisotropy{1};
    ComparisonFunc comparison_func{ComparisonFunc::never};
    float4 border_color{1.f, 1.f, 1.f, 1.f};
    float min_lod{-1000.f};
    float max_lod{1000.f};
};

class Sampler : public Object {
    KALI_OBJECT(Sampler)
public:
    Sampler(const SamplerDesc& desc, ref<Device> device);

    const SamplerDesc& get_desc() const { return m_desc; }

    gfx::ISamplerState* get_gfx_sampler() const { return m_gfx_sampler; }

    /// Returns the native API handle:
    /// - D3D12: D3D12_CPU_DESCRIPTOR_HANDLE
    /// - Vulkan: VkSampler
    NativeHandle get_native_handle() const;

private:
    SamplerDesc m_desc;
    ref<Device> m_device;
    Slang::ComPtr<gfx::ISamplerState> m_gfx_sampler;
};

} // namespace kali
