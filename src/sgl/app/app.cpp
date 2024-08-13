// SPDX-License-Identifier: Apache-2.0

#include "app.h"

#include "sgl/device/command.h"
#include "sgl/device/framebuffer.h"
#include "sgl/ui/ui.h"

namespace sgl {

// -----------------------------------------------------------------------------
// App
// -----------------------------------------------------------------------------

App::App(AppDesc desc)
{
    m_device = desc.device ? desc.device : Device::create();
}

App::~App()
{
    m_device->close();
}

void App::run()
{
    auto all_windows_should_close = [this]()
    {
        for (const auto& window : m_windows)
            if (!window->_should_close())
                return false;
        return true;
    };

    while (!m_terminate && !all_windows_should_close()) {
        run_frame();
    }
}

void App::run_frame()
{
    for (const auto& window : m_windows) {
        window->_run_frame();
    }

    m_device->run_garbage_collection();
}

void App::terminate()
{
    m_terminate = true;
}

void App::_add_window(AppWindow* window)
{
    m_windows.push_back(window);
}

void App::_remove_window(AppWindow* window)
{
    auto it = std::find(m_windows.begin(), m_windows.end(), window);
    if (it != m_windows.end())
        m_windows.erase(it);
}

// -----------------------------------------------------------------------------
// AppWindow
// -----------------------------------------------------------------------------

AppWindow::AppWindow(AppWindowDesc desc)
    : m_app(desc.app)
{
    m_device = m_app->device();
    m_window = Window::create({
        .width = desc.width,
        .height = desc.height,
        .title = desc.title,
        .mode = desc.mode,
        .resizable = desc.resizable,
    });
    m_swapchain = m_device->create_swapchain(
        {
            .format = desc.swapchain_format,
            .width = desc.width,
            .height = desc.height,
            .enable_vsync = desc.enable_vsync,
        },
        m_window
    );

    create_framebuffers();

    m_ui_context = make_ref<ui::Context>(ref(m_device));

    m_window->set_on_resize([this](uint32_t width, uint32_t height) { handle_resize(width, height); });
    m_window->set_on_keyboard_event([this](const KeyboardEvent& event) { handle_keyboard_event(event); });
    m_window->set_on_mouse_event([this](const MouseEvent& event) { handle_mouse_event(event); });
    m_window->set_on_gamepad_event([this](const GamepadEvent& event) { handle_gamepad_event(event); });
    m_window->set_on_drop_files([this](std::span<const char*> files) { handle_drop_files(files); });

    m_app->_add_window(this);
}

AppWindow::~AppWindow()
{
    m_app->_remove_window(this);
}

ui::Screen* AppWindow::screen() const
{
    return m_ui_context->screen();
}

void AppWindow::on_keyboard_event(const KeyboardEvent& event)
{
    if (event.is_key_press()) {
        if (event.key == KeyCode::escape)
            m_app->terminate();
    }
}

void AppWindow::_run_frame()
{
    m_window->process_events();
    m_ui_context->process_events();

    int image_index = m_swapchain->acquire_next_image();
    if (image_index < 0)
        return;

    ref<Texture> image = m_swapchain->get_image(image_index);
    ref<Framebuffer> framebuffer = m_framebuffers[image_index];

    ref<CommandBuffer> command_buffer = m_device->create_command_buffer();

    command_buffer->clear_texture(image, float4{0.f, 0.f, 0.f, 1.f});

    struct RenderContext render_context {
        .swapchain_image = image, .framebuffer = framebuffer, .command_buffer = command_buffer,
    };

    render(render_context);

    m_ui_context->new_frame(image->width(), image->height());

    render_ui();

    m_ui_context->render(framebuffer, command_buffer);

    command_buffer->set_texture_state(image, ResourceState::present);
    command_buffer->submit();

    image.reset();
    m_swapchain->present();
}

bool AppWindow::_should_close()
{
    return m_window->should_close();
}

void AppWindow::create_framebuffers()
{
    for (uint32_t i = 0; i < m_swapchain->desc().image_count; ++i) {
        ref<Texture> image = m_swapchain->get_image(i);
        m_framebuffers.push_back(m_device->create_framebuffer({.render_targets{image->get_rtv()}}));
    }
}

void AppWindow::handle_resize(uint32_t width, uint32_t height)
{
    m_framebuffers.clear();
    m_device->wait();
    m_swapchain->resize(width, height);
    create_framebuffers();
    on_resize(width, height);
}

void AppWindow::handle_keyboard_event(const KeyboardEvent& event)
{
    if (m_ui_context->handle_keyboard_event(event))
        return;
    on_keyboard_event(event);
}

void AppWindow::handle_mouse_event(const MouseEvent& event)
{
    if (m_ui_context->handle_mouse_event(event))
        return;
    on_mouse_event(event);
}

void AppWindow::handle_gamepad_event(const GamepadEvent& event)
{
    on_gamepad_event(event);
}

void AppWindow::handle_drop_files(std::span<const char*> files)
{
    on_drop_files(files);
}

} // namespace sgl
