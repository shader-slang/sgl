// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/types.h"
#include "sgl/device/native_handle.h"

#include "sgl/core/macros.h"
#include "sgl/core/object.h"

#include "sgl/math/vector.h"

#include <slang-gfx.h>

namespace sgl {

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

class SGL_API Sampler : public DeviceResource {
    SGL_OBJECT(Sampler)
public:
    Sampler(ref<Device> device, SamplerDesc desc);
    ~Sampler();

    const SamplerDesc& desc() const { return m_desc; }

    gfx::ISamplerState* gfx_sampler_state() const { return m_gfx_sampler_state; }

    /// Returns the native API handle:
    /// - D3D12: D3D12_CPU_DESCRIPTOR_HANDLE
    /// - Vulkan: VkSampler
    NativeHandle get_native_handle() const;

    std::string to_string() const override;

private:
    SamplerDesc m_desc;
    Slang::ComPtr<gfx::ISamplerState> m_gfx_sampler_state;
};

} // namespace sgl
