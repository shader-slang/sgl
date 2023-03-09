#pragma once

#include <kali/core/object.h>

#include <string>
#include <cstdint>

// GLFW forward declarations
struct GLFWwindow;

namespace kali {

class KALI_API Window : public Object {
public:
    Window(uint32_t width, uint32_t height, std::string title = "");
    ~Window();

    void resize(uint32_t width, uint32_t height);
    uint32_t get_width() const { return m_width; }
    uint32_t get_height() const { return m_height; }

    void set_title(std::string title);
    const std::string& get_title() const { return m_title; }

    void main_loop();

private:
    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
    GLFWwindow* m_window;
};

} // namespace kali
