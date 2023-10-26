#include "nanobind.h"

#include "kali/core/timer.h"

KALI_PY_EXPORT(core_timer)
{
    using namespace kali;

    nb::class_<Timer>(m, "Timer")
        .def(nb::init<>())
        .def("reset", &Timer::reset)
        .def("elapsed_s", &Timer::elapsed_s)
        .def("elapsed_ms", &Timer::elapsed_ms)
        .def("elapsed_us", &Timer::elapsed_us)
        .def("elapsed_ns", &Timer::elapsed_ns)
        .def_static("delta_s", &Timer::delta_s)
        .def_static("delta_ms", &Timer::delta_ms)
        .def_static("delta_us", &Timer::delta_us)
        .def_static("delta_ns", &Timer::delta_ns)
        .def_static("now", &Timer::now);
}
