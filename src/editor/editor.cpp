#include "core/window.h"
#include "core/version.h"

#include <iostream>

using namespace kali;

int main()
{
    std::cout << get_version().long_tag << std::endl;

    ref<Window> window = new Window(1024, 1024, "Editor");

    window->main_loop();

    return 0;
}
