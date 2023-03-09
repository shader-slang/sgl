#include <nanobind/nanobind.h>

int add(int a, int b)
{
    return a + b;
}

NB_MODULE(core, m)
{
    m.def("add", &add);
}
