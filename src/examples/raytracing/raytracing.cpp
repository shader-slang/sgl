// SPDX-License-Identifier: Apache-2.0

#include "kali/kali.h"
#include "kali/core/platform.h"
#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/command.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/shader_object.h"
#include "kali/device/kernel.h"
#include "kali/device/agility_sdk.h"
#include "kali/utils/tev.h"

KALI_EXPORT_AGILITY_SDK

static const std::filesystem::path EXAMPLE_DIR(KALI_EXAMPLE_DIR);

using namespace kali;

int main()
{
    kali::static_init();

    {
        ref<Device> device = Device::create({.enable_debug_layers = true});

        std::vector<float3> vertices{{-1, -1, 0}, {1, -1, 0}, {0, 1, 0}};
        std::vector<uint32_t> indices{0, 1, 2};

        ref<Buffer> vertex_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "vertex_buffer",
            .data = vertices.data(),
            .data_size = vertices.size() * sizeof(float3),
        });

        ref<Buffer> index_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "index_buffer",
            .data = indices.data(),
            .data_size = indices.size() * sizeof(uint32_t),
        });

        float3x4 identity_transform = float3x4::identity();
        ref<Buffer> transform_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "transform_buffer",
            .data = &identity_transform,
            .data_size = sizeof(identity_transform),
        });

        RayTracingGeometryDesc blas_geometry_desc{
            .type = RayTracingGeometryType::triangles,
            .flags = RayTracingGeometryFlags::opaque,
            .triangles{
                .transform3x4 = transform_buffer->device_address(),
                .index_format = Format::r32_uint,
                .vertex_format = Format::rgb32_float,
                .index_count = narrow_cast<uint32_t>(indices.size()),
                .vertex_count = narrow_cast<uint32_t>(vertices.size()),
                .index_data = index_buffer->device_address(),
                .vertex_data = vertex_buffer->device_address(),
                .vertex_stride = sizeof(float3),
            }};

        AccelerationStructureBuildInputs blas_build_inputs{
            .kind = AccelerationStructureKind::bottom_level,
            .flags = AccelerationStructureBuildFlags::none,
            .desc_count = 1,
            .geometry_descs = &blas_geometry_desc,
        };

        AccelerationStructurePrebuildInfo blas_prebuild_info
            = device->get_acceleration_structure_prebuild_info(blas_build_inputs);

        ref<Buffer> blas_scratch_buffer = device->create_buffer({
            .size = blas_prebuild_info.scratch_data_size,
            .usage = ResourceUsage::unordered_access,
            .debug_name = "blas_scratch_buffer",
        });

        ref<Buffer> blas_buffer = device->create_buffer({
            .size = blas_prebuild_info.result_data_max_size,
            .initial_state = ResourceState::acceleration_structure,
            .usage = ResourceUsage::acceleration_structure,
            .debug_name = "blas_buffer",
        });

        ref<AccelerationStructure> blas = device->create_acceleration_structure({
            .kind = AccelerationStructureKind::bottom_level,
            .buffer = blas_buffer,
            .size = blas_buffer->size(),
        });

        {
            ref<CommandBuffer> command_buffer = device->create_command_buffer();
            auto encoder = command_buffer->encode_ray_tracing_commands();
            encoder.build_acceleration_structure({
                .inputs = blas_build_inputs,
                .dst = blas,
                .scratch_data = blas_scratch_buffer->device_address(),
            });
            command_buffer->submit();
        }

        RayTracingInstanceDesc instance_desc{
            .transform = identity_transform,
            .instance_id = 0,
            .instance_mask = 0xff,
            .instance_contribution_to_hit_group_index = 0,
            .flags_ = 0,
            .acceleration_structure = blas->device_address(),
        };

        ref<Buffer> instance_buffer = device->create_structured_buffer({
            .element_count = 1,
            .struct_size = sizeof(RayTracingInstanceDesc),
            .usage = ResourceUsage::shader_resource,
            .debug_name = "instance_buffer",
            .data = &instance_desc,
            .data_size = sizeof(instance_desc),
        });

        AccelerationStructureBuildInputs tlas_build_inputs{
            .kind = AccelerationStructureKind::top_level,
            .flags = AccelerationStructureBuildFlags::none,
            .desc_count = 1,
            .instance_descs = instance_buffer->device_address(),
        };

        AccelerationStructurePrebuildInfo tlas_prebuild_info
            = device->get_acceleration_structure_prebuild_info(tlas_build_inputs);

        ref<Buffer> tlas_scratch_buffer = device->create_buffer({
            .size = tlas_prebuild_info.scratch_data_size,
            .usage = ResourceUsage::unordered_access,
            .debug_name = "tlas_scratch_buffer",
        });

        ref<Buffer> tlas_buffer = device->create_buffer({
            .size = tlas_prebuild_info.result_data_max_size,
            .initial_state = ResourceState::acceleration_structure,
            .usage = ResourceUsage::acceleration_structure,
            .debug_name = "tlas_buffer",
        });

        ref<AccelerationStructure> tlas = device->create_acceleration_structure({
            .kind = AccelerationStructureKind::top_level,
            .buffer = tlas_buffer,
            .size = tlas_buffer->size(),
        });

        {
            ref<CommandBuffer> command_buffer = device->create_command_buffer();
            auto encoder = command_buffer->encode_ray_tracing_commands();
            encoder.build_acceleration_structure({
                .inputs = tlas_build_inputs,
                .dst = tlas,
                .scratch_data = tlas_scratch_buffer->device_address(),
            });
            command_buffer->submit();
        }

        ref<Texture> render_texture = device->create_texture({
            .format = Format::rgba32_float,
            .width = 1024,
            .height = 1024,
            .usage = ResourceUsage::unordered_access,
            .debug_name = "render_texture",
        });

        ref<ComputeKernel> kernel
            = device->load_module(EXAMPLE_DIR / "raytracing.slang")->create_compute_kernel("main");

        kernel->dispatch(
            uint3{1024, 1024, 1},
            [&](ShaderCursor cursor)
            {
                cursor["tlas"] = tlas;
                cursor["render_texture"] = render_texture;
            }
        );

        utils::show_in_tev(render_texture, "raytracing");
    }

    kali::static_shutdown();
    return 0;
}
