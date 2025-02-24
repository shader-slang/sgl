// SPDX-License-Identifier: Apache-2.0

#include "raytracing.h"

#include "sgl/device/device.h"
#include "sgl/device/helpers.h"
#include "sgl/device/shader.h"

#include "sgl/core/type_utils.h"
#include "sgl/core/short_vector.h"

namespace sgl {

AccelerationStructure::AccelerationStructure(ref<Device> device, AccelerationStructureDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    rhi::AccelerationStructureDesc rhi_desc{
        .size = m_desc.size,
        .label = m_desc.label.c_str(),
    };
    SLANG_CALL(m_device->rhi_device()->createAccelerationStructure(rhi_desc, m_rhi_acceleration_structure.writeRef()));
}

AccelerationStructure::~AccelerationStructure()
{
}

DeviceAddress AccelerationStructure::device_address() const
{
    return m_rhi_acceleration_structure->getDeviceAddress();
}

std::string AccelerationStructure::to_string() const
{
    return fmt::format(
        "AccelerationStructure(\n"
        "  device = {},\n"
        "  size = {},\n"
        "  label = {}\n",
        ")",
        m_device,
        m_desc.size,
        m_desc.label
    );
}

ShaderTable::ShaderTable(ref<Device> device, ShaderTableDesc desc)
    : DeviceResource(std::move(device))
{
    short_vector<const char*, 16> rhi_ray_gen_entry_points;
    rhi_ray_gen_entry_points.reserve(desc.ray_gen_entry_points.size());
    for (const auto& name : desc.ray_gen_entry_points)
        rhi_ray_gen_entry_points.push_back(name.c_str());

    short_vector<const char*, 16> rhi_miss_entry_points;
    rhi_miss_entry_points.reserve(desc.miss_entry_points.size());
    for (const auto& name : desc.miss_entry_points)
        rhi_miss_entry_points.push_back(name.c_str());

    short_vector<const char*, 16> rhi_hit_group_names;
    rhi_hit_group_names.reserve(desc.hit_group_names.size());
    for (const auto& name : desc.hit_group_names)
        rhi_hit_group_names.push_back(name.c_str());

    short_vector<const char*, 16> rhi_callable_names;
    rhi_callable_names.reserve(desc.callable_entry_points.size());
    for (const auto& name : desc.callable_entry_points)
        rhi_callable_names.push_back(name.c_str());

    rhi::ShaderTableDesc rhi_desc{
        .rayGenShaderCount = narrow_cast<uint32_t>(rhi_ray_gen_entry_points.size()),
        .rayGenShaderEntryPointNames = rhi_ray_gen_entry_points.data(),
        .rayGenShaderRecordOverwrites = nullptr,
        .missShaderCount = narrow_cast<uint32_t>(rhi_miss_entry_points.size()),
        .missShaderEntryPointNames = rhi_miss_entry_points.data(),
        .missShaderRecordOverwrites = nullptr,
        .hitGroupCount = narrow_cast<uint32_t>(rhi_hit_group_names.size()),
        .hitGroupNames = rhi_hit_group_names.data(),
        .hitGroupRecordOverwrites = nullptr,
        .callableShaderCount = narrow_cast<uint32_t>(rhi_callable_names.size()),
        .callableShaderEntryPointNames = rhi_callable_names.data(),
        .callableShaderRecordOverwrites = nullptr,
        .program = desc.program->rhi_shader_program(),
    };

    SLANG_CALL(m_device->rhi_device()->createShaderTable(rhi_desc, m_rhi_shader_table.writeRef()));
}

ShaderTable::~ShaderTable() { }

std::string ShaderTable::to_string() const
{
    return fmt::format(
        "ShaderTable(\n"
        "  device = {}\n"
        ")",
        m_device
    );
}

} // namespace sgl
