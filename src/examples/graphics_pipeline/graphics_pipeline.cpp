// SPDX-License-Identifier: Apache-2.0

#include "sgl/sgl.h"
#include "sgl/core/platform.h"
#include "sgl/device/device.h"
#include "sgl/device/shader.h"
#include "sgl/device/command.h"
#include "sgl/device/shader_cursor.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/kernel.h"
#include "sgl/device/agility_sdk.h"
#include "sgl/device/input_layout.h"
#include "sgl/device/framebuffer.h"
#include "sgl/device/pipeline.h"
#include "sgl/utils/tev.h"

SGL_EXPORT_AGILITY_SDK

static const std::filesystem::path EXAMPLE_DIR(SGL_EXAMPLE_DIR);

using namespace sgl;

int main()
{
    sgl::static_init();

    {
        ref<Device> device = Device::create({
            .enable_debug_layers = true,
            .compiler_options = {.include_paths = {EXAMPLE_DIR}},
        });

        std::vector<float2> vertices{{-1.f, -1.f}, {1.f, -1.f}, {0.f, 1.f}};
        std::vector<uint32_t> indices{0, 1, 2};

        ref<Buffer> vertex_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "vertex_buffer",
            .data = vertices.data(),
            .data_size = vertices.size() * sizeof(float2),
        });

        ref<Buffer> index_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "index_buffer",
            .data = indices.data(),
            .data_size = indices.size() * sizeof(uint32_t),
        });

        ref<Texture> render_texture = device->create_texture({
            .format = Format::rgba32_float,
            .width = 1024,
            .height = 1024,
            .usage = ResourceUsage::render_target,
            .debug_name = "render_texture",
        });

        ref<InputLayout> input_layout = device->create_input_layout({
            .input_elements{
                {
                    .semantic_name = "POSITION",
                    .semantic_index = 0,
                    .format = Format::rg32_float,
                },
            },
            .vertex_streams{
                {.stride = 8},
            },
        });

        ref<Framebuffer> framebuffer = device->create_framebuffer({.render_targets{
            {.texture = render_texture},
        }});

        ref<ShaderProgram> program = device->load_program("graphics_pipeline.slang", {"vertex_main", "fragment_main"});
        ref<GraphicsPipeline> pipeline = device->create_graphics_pipeline({
            .program = program,
            .input_layout = input_layout,
            .framebuffer = framebuffer,
        });

        ref<CommandBuffer> command_buffer = device->create_command_buffer();
        {
            auto encoder = command_buffer->encode_render_commands(framebuffer);
            encoder.bind_pipeline(pipeline);
            encoder.set_vertex_buffer(0, vertex_buffer);
            encoder.set_primitive_topology(sgl::PrimitiveTopology::triangle_list);
            encoder.set_viewport_and_scissor_rect({
                .width = float(render_texture->width()),
                .height = float(render_texture->height()),
            });
            encoder.draw(3);
        }
        command_buffer->submit();

        utils::show_in_tev(render_texture, "graphics_pipeline");
    }

    sgl::static_shutdown();
    return 0;
}
