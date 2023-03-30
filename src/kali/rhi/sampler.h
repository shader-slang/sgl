#pragma once

#include "fwd.h"

#include "core/platform.h"
#include "core/object.h"
#include "core/vector_types.h"

#include <slang-gfx.h>

namespace kali {

enum class ComparisonFunc {
    never,
    less,
    equal,
    less_equal,
    greater,
    not_equal,
    greater_equal,
    always,
};

enum class TextureFilteringMode {
    point,
    linear,
};

enum class TextureAddressingMode {
    wrap,
    clamp_to_edge,
    clamp_to_border,
    mirror_repeat,
    mirror_once,
};

enum class TextureReductionOp {
    average,
    comparison,
    minimum,
    maximum,
};

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
public:
    Sampler(const SamplerDesc& desc, ref<Device> device);

    const SamplerDesc& get_desc() const { return m_desc; }

    gfx::ISamplerState* get_gfx_sampler() const { return m_gfx_sampler; }

private:
    SamplerDesc m_desc;
    ref<Device> m_device;
    Slang::ComPtr<gfx::ISamplerState> m_gfx_sampler;
};

} // namespace kali
