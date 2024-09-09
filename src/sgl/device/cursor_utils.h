#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/reflection.h"

namespace sgl {

bool allow_scalar_conversion(TypeReflection::ScalarType from, TypeReflection::ScalarType to);

size_t get_scalar_type_size(TypeReflection::ScalarType type);


} // namespace sgl
