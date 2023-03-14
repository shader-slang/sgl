#pragma once

#include "platform.h"
#include "object.h"

#include <string>
#include <functional>
#include <cstdint>

// GLFW forward declarations
struct GLFWwindow;

namespace kali {

class KALI_API Window : public Object {
public:
    Window(uint32_t width, uint32_t height, std::string title = "");
    ~Window();

    WindowHandle get_window_handle() const;

    void resize(uint32_t width, uint32_t height);
    uint32_t get_width() const { return m_width; }
    uint32_t get_height() const { return m_height; }

    // void set_position(uint2 position);
    // uint2 get_position() const { return m_position; }

    void set_title(std::string title);
    const std::string& get_title() const { return m_title; }

    void main_loop();

    // events

    using ResizeCallback = std::function<void(uint32_t /* width */, uint32_t /* height */)>;

    void set_on_resize(ResizeCallback on_resize) { m_on_resize = on_resize; }

private:
    void handle_window_size(uint32_t width, uint32_t height);

    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
    GLFWwindow* m_window;

    ResizeCallback m_on_resize;

    friend struct EventHandlers;
};

} // namespace kali
