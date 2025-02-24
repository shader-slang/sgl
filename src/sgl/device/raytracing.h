// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/resource.h"

#include "sgl/math/vector_types.h"
#include "sgl/math/matrix_types.h"

#include "sgl/core/macros.h"
#include "sgl/core/object.h"
#include "sgl/core/enum.h"

#include <slang-rhi.h>

namespace sgl {

struct AccelerationStructureQueryDesc {
    QueryType query_type;
    QueryPool* query_pool;
    uint32_t first_query_index;
};

struct AccelerationStructureBuildDesc {
    // TODO(slang-rhi)
    // AccelerationStructureBuildInputs inputs;
    AccelerationStructure* src;
    AccelerationStructure* dst;
    DeviceAddress scratch_data;
};

struct AccelerationStructureDesc {
    DeviceSize size{0};
    std::string label;
};

class SGL_API AccelerationStructure : public DeviceResource {
    SGL_OBJECT(AccelerationStructure)
public:
    AccelerationStructure(ref<Device> device, AccelerationStructureDesc desc);
    ~AccelerationStructure();

    const AccelerationStructureDesc& desc() const { return m_desc; }

    DeviceAddress device_address() const;

    rhi::IAccelerationStructure* rhi_acceleration_structure() const { return m_rhi_acceleration_structure; }

    std::string to_string() const override;

private:
    AccelerationStructureDesc m_desc;
    Slang::ComPtr<rhi::IAccelerationStructure> m_rhi_acceleration_structure;
};

struct ShaderTableDesc {
    ref<ShaderProgram> program;
    std::vector<std::string> ray_gen_entry_points;
    std::vector<std::string> miss_entry_points;
    std::vector<std::string> hit_group_names;
    std::vector<std::string> callable_entry_points;
};

class SGL_API ShaderTable : public DeviceResource {
    SGL_OBJECT(ShaderTable)
public:
    ShaderTable(ref<Device> device, ShaderTableDesc desc);
    ~ShaderTable();

    rhi::IShaderTable* rhi_shader_table() const { return m_rhi_shader_table; }

    std::string to_string() const override;

private:
    Slang::ComPtr<rhi::IShaderTable> m_rhi_shader_table;
};

} // namespace sgl
