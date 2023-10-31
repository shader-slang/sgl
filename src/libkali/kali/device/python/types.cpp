#include "nanobind.h"

#include "kali/device/types.h"

KALI_PY_EXPORT(device_types)
{
    using namespace kali;

    nb::kali_enum<ShaderModel>(m, "ShaderModel");
    nb::kali_enum<ShaderStage>(m, "ShaderStage");

    nb::kali_enum<ComparisonFunc>(m, "ComparisonFunc");
    nb::kali_enum<TextureFilteringMode>(m, "TextureFilteringMode");
    nb::kali_enum<TextureAddressingMode>(m, "TextureAddressingMode");
    nb::kali_enum<TextureReductionOp>(m, "TextureReductionOp");
}
