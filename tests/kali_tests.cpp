#include "testing.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    doctest::Context context;

    // context.setOption("success", true);

    return context.run();
}
