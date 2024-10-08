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
    gfx::IAccelerationStructure::CreateDesc gfx_desc{
        .kind = static_cast<gfx::IAccelerationStructure::Kind>(m_desc.kind),
        .buffer = m_desc.buffer->gfx_buffer_resource(),
        .offset = m_desc.offset,
        .size = m_desc.size,
    };
    SLANG_CALL(m_device->gfx_device()->createAccelerationStructure(gfx_desc, m_gfx_acceleration_structure.writeRef()));
}

AccelerationStructure::~AccelerationStructure()
{
    m_device->deferred_release(m_gfx_acceleration_structure);
}

DeviceAddress AccelerationStructure::device_address() const
{
    return m_gfx_acceleration_structure->getDeviceAddress();
}

std::string AccelerationStructure::to_string() const
{
    return fmt::format(
        "AccelerationStructure(\n"
        "  device = {},\n"
        "  kind = {},\n"
        "  buffer = {},\n"
        "  offset = {},\n"
        "  size = {}\n"
        ")",
        m_device,
        m_desc.kind,
        m_desc.buffer,
        m_desc.offset,
        m_desc.size
    );
}

ShaderTable::ShaderTable(ref<Device> device, ShaderTableDesc desc)
    : DeviceResource(std::move(device))
{
    short_vector<const char*, 16> gfx_ray_gen_entry_points;
    gfx_ray_gen_entry_points.reserve(desc.ray_gen_entry_points.size());
    for (const auto& name : desc.ray_gen_entry_points)
        gfx_ray_gen_entry_points.push_back(name.c_str());

    short_vector<const char*, 16> gfx_miss_entry_points;
    gfx_miss_entry_points.reserve(desc.miss_entry_points.size());
    for (const auto& name : desc.miss_entry_points)
        gfx_miss_entry_points.push_back(name.c_str());

    short_vector<const char*, 16> gfx_hit_group_names;
    gfx_hit_group_names.reserve(desc.hit_group_names.size());
    for (const auto& name : desc.hit_group_names)
        gfx_hit_group_names.push_back(name.c_str());

    short_vector<const char*, 16> gfx_callable_names;
    gfx_callable_names.reserve(desc.callable_entry_points.size());
    for (const auto& name : desc.callable_entry_points)
        gfx_callable_names.push_back(name.c_str());

    gfx::IShaderTable::Desc gfx_desc{
        .rayGenShaderCount = narrow_cast<gfx::GfxCount>(gfx_ray_gen_entry_points.size()),
        .rayGenShaderEntryPointNames = gfx_ray_gen_entry_points.data(),
        .rayGenShaderRecordOverwrites = nullptr,
        .missShaderCount = narrow_cast<gfx::GfxCount>(gfx_miss_entry_points.size()),
        .missShaderEntryPointNames = gfx_miss_entry_points.data(),
        .missShaderRecordOverwrites = nullptr,
        .hitGroupCount = narrow_cast<gfx::GfxCount>(gfx_hit_group_names.size()),
        .hitGroupNames = gfx_hit_group_names.data(),
        .hitGroupRecordOverwrites = nullptr,
        // .callableShaderCount = narrow_cast<gfx::GfxCount>(gfx_callable_names.size()),
        // .callableShaderEntryPointNames = gfx_callable_names.data(),
        // .callableShaderRecordOverwrites = nullptr,
        .program = desc.program->gfx_shader_program(),
    };

    SLANG_CALL(m_device->gfx_device()->createShaderTable(gfx_desc, m_gfx_shader_table.writeRef()));
}

ShaderTable::~ShaderTable()
{
    m_device->deferred_release(m_gfx_shader_table);
}

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
