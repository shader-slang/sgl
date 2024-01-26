#include "kali/kali.h"
#include "kali/core/platform.h"
#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/command.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/shader_object.h"
#include "kali/device/kernel.h"
#include "kali/device/agility_sdk.h"
#include "kali/device/input_layout.h"
#include "kali/device/framebuffer.h"
#include "kali/device/pipeline.h"
#include "kali/utils/tev.h"

KALI_EXPORT_AGILITY_SDK

static const std::filesystem::path EXAMPLE_DIR(KALI_EXAMPLE_DIR);

using namespace kali;

int main()
{
    kali::static_init();

    {
        ref<Device> device = Device::create({.enable_debug_layers = true});

        std::vector<float2> vertices{{-1.f, -1.f}, {1.f, -1.f}, {0.f, 1.f}};
        std::vector<uint32_t> indices{0, 1, 2};

        ref<Buffer> vertex_buffer = device->create_buffer(
            {
                .usage = ResourceUsage::shader_resource,
                .debug_name = "vertex_buffer",
            },
            vertices.data(),
            vertices.size() * sizeof(float2)
        );

        ref<Buffer> index_buffer = device->create_buffer(
            {
                .usage = ResourceUsage::shader_resource,
                .debug_name = "index_buffer",
            },
            indices.data(),
            indices.size() * sizeof(uint32_t)
        );

        ref<Texture> render_texture = device->create_texture({
            .type = TextureType::texture_2d,
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

        ref<SlangModule> module_ = device->load_module(EXAMPLE_DIR / "graphics_pipeline.slang");
        auto vs = module_->entry_point("vertex_main");
        auto fs = module_->entry_point("fragment_main");
        ref<ShaderProgram> program = module_->create_program(module_->global_scope(), {vs, fs});
        ref<GraphicsPipeline> pipeline = device->create_graphics_pipeline({
            .program = program,
            .input_layout = input_layout,
            .framebuffer = framebuffer,
        });

        ref<CommandBuffer> command_buffer = device->create_command_buffer();
        auto encoder = command_buffer->encode_render_commands(framebuffer);
        encoder.bind_pipeline(pipeline);
        encoder.set_vertex_buffer(0, vertex_buffer);
        encoder.set_primitive_topology(kali::PrimitiveTopology::triangle_list);
        encoder.set_viewport_and_scissor_rect({
            .width = float(render_texture->width()),
            .height = float(render_texture->height()),
        });
        encoder.draw(3);
        command_buffer->submit();

        utils::show_in_tev(render_texture, "graphics_pipeline");
    }

    kali::static_shutdown();
    return 0;
}
