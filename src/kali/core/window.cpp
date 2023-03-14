#include "window.h"
#include "error.h"

#include <GLFW/glfw3.h>

#if KALI_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif KALI_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#endif
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include <atomic>

namespace kali {

namespace {
    std::atomic<uint32_t> glfw_ref_count{0};

    inline void init_glfw()
    {
        if (glfw_ref_count.fetch_add(1) == 0)
            if (glfwInit() != GLFW_TRUE)
                KALI_THROW(Exception("Failed to initialize GLFW"));
    }

    inline void terminate_glfw()
    {
        KALI_ASSERT(glfw_ref_count > 0);
        if (glfw_ref_count.fetch_sub(1) == 1)
            glfwTerminate();
    }

} // namespace

struct EventHandlers {
    static void handle_error(int error_code, const char* description)
    {
        KALI_ERROR("GLFW error {}: {}", error_code, description);
    }

    static void handle_window_size(GLFWwindow* window, int width, int height)
    {
        static_cast<Window*>(glfwGetWindowUserPointer(window))->handle_window_size(width, height);
    }


};


Window::Window(uint32_t width, uint32_t height, std::string title)
    : m_width(width)
    , m_height(height)
    , m_title(title)
{
    init_glfw();

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
    if (!m_window)
        KALI_THROW(Exception("Failed to create GLFW window"));

    glfwSetWindowUserPointer(m_window, this);
    glfwSetErrorCallback(&EventHandlers::handle_error);
    glfwSetWindowSizeCallback(m_window, &EventHandlers::handle_window_size);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);

    terminate_glfw();
}

WindowHandle Window::get_window_handle() const
{
    WindowHandle handle{};
#if KALI_WINDOWS
    handle = glfwGetWin32Window(m_window);
#elif KALI_LINUX
    handle.xdisplay = glfwGetX11Display(m_window);
    handle.xwindow = glfwGetX11Window(m_window);
#endif
    return handle;
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

void Window::handle_window_size(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    if (m_on_resize)
        m_on_resize(m_width, m_height);
}

} // namespace kali
