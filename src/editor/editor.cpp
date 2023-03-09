#include <kali/core/window.h>

using namespace kali;

int main()
{
    ref<Window> window = new Window(1024, 1024, "Editor");

    window->main_loop();

    return 0;
}
