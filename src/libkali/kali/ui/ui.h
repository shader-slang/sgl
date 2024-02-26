// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/ui/fwd.h"

#include "kali/core/fwd.h"
#include "kali/core/object.h"
#include "kali/core/timer.h"

#include "kali/device/fwd.h"

#include <map>

struct ImGuiContext;
struct ImFont;

namespace kali::ui {

class KALI_API Context : public Object {
    KALI_OBJECT(Context)
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

    // TODO: we can reuse pipelines for the same framebuffer layout
    std::map<Framebuffer*, ref<GraphicsPipeline>> m_pipelines;
};

} // namespace kali::ui

// Extend ImGui with some additional convenience functions.
namespace ImGui {

KALI_API void PushFont(const char* name);

}
