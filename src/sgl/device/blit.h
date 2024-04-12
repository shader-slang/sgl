// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"

#include "sgl/core/object.h"

#include "sgl/math/vector_types.h"

#include <map>

namespace sgl {

/**
 * \brief Utility class for blitting textures and generating mipmaps.
 *
 * Because compute shaders have limited support for UAV texture formats,
 * this class uses a graphics pipeline to blit a texture to a render target.
 */
class Blitter : public Object {
    SGL_OBJECT(Blitter)
public:
    Blitter(Device* device);
    ~Blitter();

    /**
     * \brief Blit a texture (SRV) to another texture (RTV).
     *
     * Blits the full extent of the source texture to the destination texture.
     *
     * \param command_buffer Command buffer.
     * \param dst RTV of the destination texture.
     * \param src SRV of the source texture.
     * \param filter Filtering mode to use.
     */
    void blit(
        CommandBuffer* command_buffer,
        ResourceView* dst,
        ResourceView* src,
        TextureFilteringMode filter = TextureFilteringMode::linear
    );

    /**
     * \brief Generate mipmaps for a texture.
     *
     * Repetatively blits the texture to a smaller mip level until the smallest mip level is reached.
     * The texture needs to have mip levels pre-allocated and have usage flags for SRV and RTV.
     * Supports both 2D and 2D array textures.
     *
     * \param command_buffer Command buffer.
     * \param texture Texture to generate mipmaps for.
     * \param array_layer Array layer to generate mipmaps for.
     */
    void generate_mips(CommandBuffer* command_buffer, Texture* texture, uint32_t array_layer = 0);

private:
    enum class TextureType {
        float_,
        int_,
    };

    enum class TextureLayout {
        texture_2d,
        texture_2d_array,
    };

    struct ProgramKey {
        TextureLayout src_layout;
        TextureType src_type;
        TextureType dst_type;

        auto operator<=>(const ProgramKey&) const = default;
    };

    ref<ShaderProgram> get_program(ProgramKey key);
    ref<GraphicsPipeline> get_pipeline(ProgramKey key, const Framebuffer* framebuffer);

    Device* m_device;
    ref<Sampler> m_linear_sampler;
    ref<Sampler> m_point_sampler;

    std::map<ProgramKey, ref<ShaderProgram>> m_program_cache;
    std::map<std::pair<ProgramKey, FramebufferLayoutDesc>, ref<GraphicsPipeline>> m_pipeline_cache;
};

} // namespace sgl
