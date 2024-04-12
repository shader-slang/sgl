// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/app/app.h"
#include "sgl/device/command.h"
#include "sgl/device/resource.h"
#include "sgl/device/framebuffer.h"
#include "sgl/ui/widgets.h"

namespace sgl {

SGL_DICT_TO_DESC_BEGIN(AppDesc)
SGL_DICT_TO_DESC_FIELD(device, ref<Device>)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(AppWindowDesc)
SGL_DICT_TO_DESC_FIELD(width, uint32_t)
SGL_DICT_TO_DESC_FIELD(height, uint32_t)
SGL_DICT_TO_DESC_FIELD(title, std::string)
SGL_DICT_TO_DESC_FIELD(mode, WindowMode)
SGL_DICT_TO_DESC_FIELD(resizable, bool)
SGL_DICT_TO_DESC_FIELD(swapchain_format, Format)
SGL_DICT_TO_DESC_FIELD(enable_vsync, bool)
SGL_DICT_TO_DESC_END()

class PyApp : App {
public:
    NB_TRAMPOLINE(App, 10);

    PyApp(AppDesc desc)
        : App(desc)
    {
    }
};

class PyAppWindow : AppWindow {
public:
    NB_TRAMPOLINE(AppWindow, 10);

    PyAppWindow(AppWindowDesc desc)
        : AppWindow(desc)
    {
    }

    void render(RenderContext render_context) override { NB_OVERRIDE(render, render_context); }

    void on_resize(uint32_t width, uint32_t height) override { NB_OVERRIDE(on_resize, width, height); }
    void on_keyboard_event(const KeyboardEvent& event) override { NB_OVERRIDE(on_keyboard_event, event); }
    void on_mouse_event(const MouseEvent& event) override { NB_OVERRIDE(on_mouse_event, event); }
    void on_gamepad_event(const GamepadEvent& event) override { NB_OVERRIDE(on_gamepad_event, event); }
    void on_drop_files(std::span<const char*> files) override { NB_OVERRIDE(on_drop_files, files); }
};

} // namespace sgl

SGL_PY_EXPORT(app_app)
{
    using namespace sgl;

    nb::class_<AppDesc>(m, "AppDesc")
        .def(nb::init<>())
        .def("__init__", [](AppDesc* self, nb::dict dict) { new (self) AppDesc(dict_to_AppDesc(dict)); })
        .def_rw("device", &AppDesc::device, D_NA(AppDesc, device));
    nb::implicitly_convertible<nb::dict, AppDesc>();

    nb::class_<App, PyApp, Object>(m, "App")
        .def(nb::init<AppDesc>())
        .def(
            "__init__",
            [](App* self, ref<Device> device)
            {
                new (self) PyApp({
                    .device = device,
                });
            },
            "device"_a.none() = nb::none(),
            D_NA(App, App)
        )
        .def_prop_ro("device", &App::device, D_NA(App, device))
        .def("run", &App::run, D_NA(App, run))
        .def("run_frame", &App::run_frame, D_NA(App, run_frame))
        .def("terminate", &App::terminate, D_NA(App, terminate));

    nb::class_<AppWindowDesc>(m, "AppWindowDesc", D_NA(AppWindowDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](AppWindowDesc* self, nb::dict dict) { new (self) AppWindowDesc(dict_to_AppWindowDesc(dict)); }
        )
        .def_rw("width", &AppWindowDesc::width, D_NA(AppWindowDesc, width))
        .def_rw("height", &AppWindowDesc::height, D_NA(AppWindowDesc, height))
        .def_rw("title", &AppWindowDesc::title, D_NA(AppWindowDesc, title))
        .def_rw("mode", &AppWindowDesc::mode, D_NA(AppWindowDesc, mode))
        .def_rw("resizable", &AppWindowDesc::resizable, D_NA(AppWindowDesc, resizable))
        .def_rw("swapchain_format", &AppWindowDesc::swapchain_format, D_NA(AppWindowDesc, swapchain_format))
        .def_rw("enable_vsync", &AppWindowDesc::enable_vsync, D_NA(AppWindowDesc, enable_vsync));
    nb::implicitly_convertible<nb::dict, AppWindowDesc>();

    nb::class_<AppWindow, PyAppWindow, Object> app_window(m, "AppWindow", D_NA(AppWindow));

    nb::class_<AppWindow::RenderContext>(app_window, "RenderContext", D_NA(AppWindow, RenderContext))
        .def_ro(
            "swapchain_image",
            &AppWindow::RenderContext::swapchain_image,
            D_NA(AppWindow::RenderContext, swapchain_image)
        )
        .def_ro("framebuffer", &AppWindow::RenderContext::framebuffer, D_NA(AppWindow::RenderContext, framebuffer))
        .def_ro(
            "command_buffer",
            &AppWindow::RenderContext::command_buffer,
            D_NA(AppWindow::RenderContext, command_buffer)
        );

    app_window //
        .def(
            "__init__",
            [](AppWindow* self,
               App* app,
               uint32_t width,
               uint32_t height,
               std::string title,
               WindowMode mode,
               bool resizable,
               Format swapchain_format,
               bool enable_vsync)
            {
                new (self) PyAppWindow({
                    .app = app,
                    .width = width,
                    .height = height,
                    .title = title,
                    .mode = mode,
                    .resizable = resizable,
                    .swapchain_format = swapchain_format,
                    .enable_vsync = enable_vsync,
                });
            },
            "app"_a,
            "width"_a = 1920,
            "height"_a = 1280,
            "title"_a = "sgl",
            "mode"_a = WindowMode::normal,
            "resizable"_a = true,
            "swapchain_format"_a = Format::bgra8_unorm_srgb,
            "enable_vsync"_a = false,
            D_NA(App, App)
        )
        .def_prop_ro("device", &AppWindow::device, D_NA(AppWindow, device))
        .def_prop_ro("screen", &AppWindow::screen, D_NA(AppWindow, screen))
        .def("render", &AppWindow::render, "render_context"_a, D_NA(AppWindow, render))
        .def("on_resize", &AppWindow::on_resize, "width"_a, "height"_a, D_NA(AppWindow, on_resize))
        .def("on_keyboard_event", &AppWindow::on_keyboard_event, "event"_a, D_NA(AppWindow, on_keyboard_event))
        .def("on_mouse_event", &AppWindow::on_mouse_event, "event"_a, D_NA(AppWindow, on_mouse_event))
        .def("on_gamepad_event", &AppWindow::on_gamepad_event, "event"_a, D_NA(AppWindow, on_gamepad_event))
        .def("on_drop_files", &AppWindow::on_drop_files, "files"_a, D_NA(AppWindow, on_drop_files));
}
