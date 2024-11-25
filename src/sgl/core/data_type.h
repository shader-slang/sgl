// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/macros.h"
#include "sgl/core/enum.h"

namespace sgl {

enum class DataType {
    void_,
    bool_,
    int8,
    int16,
    int32,
    int64,
    uint8,
    uint16,
    uint32,
    uint64,
    float16,
    float32,
    float64,
};

SGL_ENUM_INFO(
    DataType,
    {
        {DataType::void_, "void"},
        {DataType::bool_, "bool"},
        {DataType::int8, "int8"},
        {DataType::int16, "int16"},
        {DataType::int32, "int32"},
        {DataType::int64, "int64"},
        {DataType::uint8, "uint8"},
        {DataType::uint16, "uint16"},
        {DataType::uint32, "uint32"},
        {DataType::uint64, "uint64"},
        {DataType::float16, "float16"},
        {DataType::float32, "float32"},
        {DataType::float64, "float64"},
    }
);
SGL_ENUM_REGISTER(DataType);

} // namespace sgl
