#include "renderer.h"

#include "kali/window.h"
#include "kali/version.h"
#include "kali/error.h"
#include "kali/logger.h"
#include "kali/platform.h"
#include "kali/math/vector.h"

#include <iostream>

using namespace kali;

int main()
{
    init_platform();

    Logger::global().add_file_output("editor.log");

    log_info("{}", get_version().long_tag);

    log_debug("just a test");
    log_debug("{} {} {}", "hello", 42, "world!");

    log_error_once("just a test");
    log_error_once("just a test");
    log_error_once("just a test");

    log_info("{}", uint4(1, 2, 3, 4));

    ref<Window> window = Window::create({.width = 1024, .height = 1024, .title = "Kali Editor"});

    window->set_on_resize([](uint32_t width, uint32_t height) { log_info("Window resized to {}x{}", width, height); });

    window->set_on_mouse_event(
        [](const MouseEvent& event)
        {
            switch (event.type) {
            case MouseEventType::button_down:
                log_info("mouse down");
                break;
            case MouseEventType::button_up:
                log_info("mouse up");
                break;
            case MouseEventType::move:
                log_info("mouse move");
                break;
            case MouseEventType::scroll:
                log_info("mouse scroll");
                break;
            }
        }
    );

    window->set_on_drop_files(
        [](const std::span<const char*> files)
        {
            for (const char* file : files) {
                log_info("dropped file: {}", file);
            }
        }
    );

    Renderer renderer(window);

    window->main_loop();

    return 0;
}
