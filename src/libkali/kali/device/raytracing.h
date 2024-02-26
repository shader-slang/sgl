// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/device/fwd.h"
#include "kali/device/types.h"
#include "kali/device/device_resource.h"
#include "kali/device/resource.h"

#include "kali/math/vector_types.h"
#include "kali/math/matrix_types.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <slang-gfx.h>

namespace kali {

struct AccelerationStructureQueryDesc {
    QueryType query_type;
    QueryPool* query_pool;
    uint32_t first_query_index;
};

struct AccelerationStructureBuildDesc {
    AccelerationStructureBuildInputs inputs;
    AccelerationStructure* src;
    AccelerationStructure* dst;
    DeviceAddress scratch_data;
};

struct AccelerationStructureDesc {
    AccelerationStructureKind kind;
    ref<Buffer> buffer;
    DeviceOffset offset{0};
    DeviceSize size{0};
};

class KALI_API AccelerationStructure : public DeviceResource {
    KALI_OBJECT(AccelerationStructure)
public:
    AccelerationStructure(ref<Device> device, AccelerationStructureDesc desc);
    ~AccelerationStructure();

    const AccelerationStructureDesc& desc() const { return m_desc; }

    AccelerationStructureKind kind() const { return m_desc.kind; }

    DeviceAddress device_address() const;

    gfx::IAccelerationStructure* gfx_acceleration_structure() const { return m_gfx_acceleration_structure; }

    std::string to_string() const override;

private:
    AccelerationStructureDesc m_desc;
    Slang::ComPtr<gfx::IAccelerationStructure> m_gfx_acceleration_structure;
};

struct ShaderTableDesc {
    const ShaderProgram* program;
    std::vector<std::string> ray_gen_entry_points;
    std::vector<std::string> miss_entry_points;
    std::vector<std::string> hit_group_names;
    std::vector<std::string> callable_entry_points;
};

class KALI_API ShaderTable : public DeviceResource {
    KALI_OBJECT(ShaderTable)
public:
    ShaderTable(ref<Device> device, ShaderTableDesc desc);
    ~ShaderTable();

    gfx::IShaderTable* gfx_shader_table() const { return m_gfx_shader_table; }

    std::string to_string() const override;

private:
    Slang::ComPtr<gfx::IShaderTable> m_gfx_shader_table;
};

} // namespace kali
