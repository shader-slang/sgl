#include <kali/core/window.h>
#include <kali/core/assert.h>
#include <kali/core/error.h>

#include <GLFW/glfw3.h>

#include <atomic>

namespace kali {

namespace {
    std::atomic<uint32_t> glfw_ref_count { 0 };

    inline void init_glfw()
    {
        if (glfw_ref_count.fetch_add(1) == 0)
            if (glfwInit() != GLFW_TRUE)
                KALI_THROW("Failed to initialize GLFW");
    }

    inline void terminate_glfw()
    {
        KALI_ASSERT(glfw_ref_count > 0);
        if (glfw_ref_count.fetch_sub(1) == 1)
            glfwTerminate();
    }
} // namespace

Window::Window(uint32_t width, uint32_t height, std::string title)
    : m_width(width)
    , m_height(height)
    , m_title(title)
{
    init_glfw();

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
    if (!m_window)
        KALI_THROW("Failed to create GLFW window");
}

Window::~Window()
{
    glfwDestroyWindow(m_window);

    terminate_glfw();
}

void Window::resize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    glfwSetWindowSize(m_window, m_width, m_height);
}

void Window::set_title(std::string title)
{
    m_title = std::move(title);
    glfwSetWindowTitle(m_window, m_title.c_str());
}

void Window::main_loop()
{
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }
}


} // namespace kali
