// SPDX-License-Identifier: Apache-2.0

#include "blit.h"

#include "sgl/device/device.h"
#include "sgl/device/resource.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/sampler.h"
#include "sgl/device/command.h"
#include "sgl/device/shader_cursor.h"

#include "sgl/core/error.h"

#include "sgl/math/vector.h"

namespace sgl {

Blitter::Blitter(Device* device)
    : m_device(device)
{
    m_linear_sampler = m_device->create_sampler({
        .min_filter = TextureFilteringMode::linear,
        .mag_filter = TextureFilteringMode::linear,
    });

    m_point_sampler = m_device->create_sampler({
        .min_filter = TextureFilteringMode::point,
        .mag_filter = TextureFilteringMode::point,
    });
}

Blitter::~Blitter() { }

void Blitter::blit(CommandEncoder* command_encoder, TextureView* dst, TextureView* src, TextureFilteringMode filter)
{
    SGL_UNUSED(filter);

    SGL_CHECK_NOT_NULL(command_encoder);
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);

    Texture* dst_texture = dst->texture();
    Texture* src_texture = src->texture();

    SGL_CHECK(dst_texture->type() == TextureType::texture_2d, "dst must be a 2D texture");
    SGL_CHECK(is_set(dst_texture->desc().usage, TextureUsage::render_target), "dst must be a render target");
    SGL_CHECK(src_texture->type() == TextureType::texture_2d, "src must be a 2D texture");
    SGL_CHECK(is_set(src_texture->desc().usage, TextureUsage::shader_resource), "src must be a shader resource");
    SGL_CHECK(
        dst_texture->desc().sample_count == 1 && src_texture->desc().sample_count == 1,
        "src and dst must be not be multi-sampled textures"
    );

    uint32_t dst_mip_level = dst->subresource_range().mip_level;
    uint32_t dst_base_array_layer = dst->subresource_range().base_array_layer;
    uint32_t src_mip_level = src->subresource_range().mip_level;

    uint2 dst_size = src_texture->get_mip_dimensions(dst_mip_level).xy();
    uint2 src_size = src_texture->get_mip_dimensions(src_mip_level).xy();

    auto determine_texture_type = [](Format resource_format) -> TextureDataType
    {
        const auto& info = get_format_info(resource_format);
        if (info.is_float_format() || info.is_normalized_format())
            return TextureDataType::float_;
        return TextureDataType::int_;
    };

    TextureDataType dst_type = determine_texture_type(dst_texture->format());
    TextureDataType src_type = determine_texture_type(src_texture->format());
    TextureLayout src_layout
        = src_texture->array_size() > 1 ? TextureLayout::texture_2d_array : TextureLayout::texture_2d;

    ref<RenderPipeline> pipeline = get_pipeline(
        {
            .src_layout = src_layout,
            .src_type = src_type,
            .dst_type = dst_type,
        },
        dst_texture->format()
    );

    {
        auto encoder = command_buffer->encode_render_commands(framebuffer);
        ShaderCursor cursor = ShaderCursor(encoder.bind_pipeline(pipeline));
        encoder.set_primitive_topology(PrimitiveTopology::triangle_list);
        encoder.set_viewport_and_scissor_rect({
            .width = float(dst_size.x),
            .height = float(dst_size.y),
        });
        cursor["src"] = ref(src);
        cursor["sampler"] = filter == TextureFilteringMode::linear ? m_linear_sampler : m_point_sampler;
        encoder.draw(3);
    }
}

void Blitter::generate_mips(CommandEncoder* command_encoder, Texture* texture, uint32_t array_layer)
{
    SGL_CHECK_NOT_NULL(command_encoder);
    SGL_CHECK_NOT_NULL(texture);
    SGL_CHECK_LT(array_layer, texture->array_size());

    for (uint32_t i = 0; i < texture->mip_count() - 1; ++i) {
        ref<TextureView> src = texture->create_view({
            .subresource_range{
                .mip_level = i,
                .mip_count = 1,
                .base_array_layer = array_layer,
                .layer_count = 1,
            },
        });
        ref<TextureView> dst = texture->create_view({
            .subresource_range{
                .mip_level = i + 1,
                .mip_count = 1,
                .base_array_layer = array_layer,
                .layer_count = 1,
            },
        });
        blit(command_encoder, dst, src);
    }
}

ref<ShaderProgram> Blitter::get_program(ProgramKey key)
{
    auto it = m_program_cache.find(key);
    if (it != m_program_cache.end())
        return it->second;

    std::string source;
    source += fmt::format(
        "#define SRC_LAYOUT {}\n"
        "#define SRC_TYPE {}\n"
        "#define DST_TYPE {}\n\n",
        uint32_t(key.src_layout),
        uint32_t(key.src_type),
        uint32_t(key.dst_type)
    );
    source += m_device->slang_session()->load_source("sgl/device/blit.slang");

    ref<SlangModule> module = m_device->slang_session()->load_module_from_source("blit", source);
    module->break_strong_reference_to_session();
    ref<ShaderProgram> program = m_device->slang_session()->link_program(
        {module},
        {
            module->entry_point("vs_main"),
            module->entry_point("fs_main"),
        }
    );

    m_program_cache[key] = program;
    return program;
}

ref<RenderPipeline> Blitter::get_pipeline(ProgramKey key, Format dst_format)
{
    auto it = m_pipeline_cache.find({key, dst_format});
    if (it != m_pipeline_cache.end())
        return it->second;

    ref<ShaderProgram> program = get_program(key);

    ref<RenderPipeline> pipeline = m_device->create_render_pipeline({
        .program = program,
        .targets = {
            {
                .format = dst_format,
            },
        },
    });

    m_pipeline_cache[{key, dst_format}] = pipeline;
    return pipeline;
}

} // namespace sgl
