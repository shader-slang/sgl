// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/ui/fwd.h"

#include "sgl/core/fwd.h"
#include "sgl/core/object.h"
#include "sgl/core/timer.h"

#include "sgl/device/fwd.h"
#include "sgl/device/framebuffer.h"

#include <map>

struct ImGuiContext;
struct ImFont;

namespace sgl::ui {

class SGL_API Context : public Object {
    SGL_OBJECT(Context)
public:
    Context(ref<Device> device);
    ~Context();

    ref<Screen> screen() const { return m_screen; }

    ImFont* get_font(const char* name);

    void new_frame(uint32_t width, uint32_t height);
    void render(Framebuffer* framebuffer, CommandBuffer* command_buffer);

    bool handle_keyboard_event(const KeyboardEvent& event);
    bool handle_mouse_event(const MouseEvent& event);

    void process_events();

private:
    GraphicsPipeline* get_pipeline(Framebuffer* framebuffer);

    static constexpr uint32_t FRAME_COUNT = 3;

    ref<Device> m_device;
    ImGuiContext* m_imgui_context;

    ref<Screen> m_screen;

    uint32_t m_frame_index{0};
    Timer m_frame_timer;

    ref<Sampler> m_sampler;
    ref<Buffer> m_vertex_buffers[FRAME_COUNT];
    ref<Buffer> m_index_buffers[FRAME_COUNT];
    ref<ShaderProgram> m_program;
    ref<Texture> m_font_texture;
    ref<InputLayout> m_input_layout;

    std::map<std::string, ImFont*> m_fonts;

    std::map<FramebufferLayoutDesc, ref<GraphicsPipeline>> m_pipelines;
};

} // namespace sgl::ui

// Extend ImGui with some additional convenience functions.
namespace ImGui {

SGL_API void PushFont(const char* name);

}
