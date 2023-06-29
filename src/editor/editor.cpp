#include "renderer.h"

#include "core/window.h"
#include "core/version.h"
#include "core/error.h"
#include "core/logger.h"
#include "core/vector_ops.h"
#include "core/platform.h"

#include <iostream>

using namespace kali;

namespace nested {

struct Test {
    static void test() { std::cout << format_stacktrace(backtrace()); }
};


} // namespace nested

int main()
{
    init_platform();

    KALI_INFO("{}", get_version().long_tag);

    KALI_DEBUG("just a test");
    KALI_DEBUG("{} {} {}", "hello", 42, "world!");

    KALI_DEBUG_WITH_LOC("just a test");
    KALI_DEBUG_WITH_LOC("{} {} {}", "hello", 42, "world!");

    KALI_INFO("{}", to_string(uint4(1, 2, 3, 4)));

    nested::Test::test();

    ref<Window> window = Window::create({.width = 1024, .height = 1024, .title = "Kali Editor"});

    window->set_on_resize([](uint32_t width, uint32_t height) { KALI_INFO("Window resized to {}x{}", width, height); });

    window->set_on_mouse_event(
        [](const MouseEvent& event)
        {
            switch (event.type) {
            case MouseEventType::button_down:
                KALI_INFO("mouse down");
                break;
            case MouseEventType::button_up:
                KALI_INFO("mouse up");
                break;
            case MouseEventType::move:
                KALI_INFO("mouse move");
                break;
            case MouseEventType::scroll:
                KALI_INFO("mouse scroll");
                break;
            }
        }
    );

    window->set_on_drop_files(
        [](const std::span<const char*> files)
        {
            for (const char* file : files) {
                KALI_INFO("dropped file: {}", file);
            }
        }
    );

    Renderer renderer(window);

    window->main_loop();

    return 0;
}
