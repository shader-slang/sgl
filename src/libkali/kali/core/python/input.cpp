#include "nanobind.h"

#include "kali/core/input.h"

KALI_PY_EXPORT(core_input)
{
    using namespace kali;

    nb::kali_enum<MouseButton>(m, "MouseButton");
    nb::kali_enum<KeyModifierFlags>(m, "KeyModifierFlags");
    nb::kali_enum<KeyModifier>(m, "KeyModifier");
    nb::kali_enum<KeyCode>(m, "KeyCode");

    nb::kali_enum<KeyboardEventType>(m, "KeyboardEventType");

    nb::class_<KeyboardEvent>(m, "KeyboardEvent")
        .def_ro("type", &KeyboardEvent::type)
        .def_ro("key", &KeyboardEvent::key)
        .def_ro("mods", &KeyboardEvent::mods)
        .def_ro("codepoint", &KeyboardEvent::codepoint)
        .def("is_key_press", &KeyboardEvent::is_key_press)
        .def("is_key_release", &KeyboardEvent::is_key_release)
        .def("is_key_repeat", &KeyboardEvent::is_key_repeat)
        .def("is_input", &KeyboardEvent::is_input)
        .def("has_modifier", &KeyboardEvent::has_modifier)
        .def("__repr__", &KeyboardEvent::to_string);

    nb::kali_enum<MouseEventType>(m, "MouseEventType");

    nb::class_<MouseEvent>(m, "MouseEvent")
        .def_ro("type", &MouseEvent::type)
        .def_ro("pos", &MouseEvent::pos)
        .def_ro("scroll", &MouseEvent::scroll)
        .def_ro("button", &MouseEvent::button)
        .def_ro("mods", &MouseEvent::mods)
        .def("is_button_down", &MouseEvent::is_button_down)
        .def("is_button_up", &MouseEvent::is_button_up)
        .def("is_move", &MouseEvent::is_move)
        .def("is_scroll", &MouseEvent::is_scroll)
        .def("has_modifier", &MouseEvent::has_modifier)
        .def("__repr__", &MouseEvent::to_string);

    nb::kali_enum<GamepadEventType>(m, "GamepadEventType");
    nb::kali_enum<GamepadButton>(m, "GamepadButton");

    nb::class_<GamepadEvent>(m, "GamepadEvent")
        .def_ro("type", &GamepadEvent::type)
        .def_ro("button", &GamepadEvent::button)
        .def("__repr__", &GamepadEvent::to_string);

    nb::class_<GamepadState>(m, "GamepadState")
        .def_ro("left_x", &GamepadState::left_x)
        .def_ro("left_y", &GamepadState::left_y)
        .def_ro("right_x", &GamepadState::right_x)
        .def_ro("right_y", &GamepadState::right_y)
        .def_ro("left_trigger", &GamepadState::left_trigger)
        .def_ro("right_trigger", &GamepadState::right_trigger)
        .def_ro("buttons", &GamepadState::buttons)
        .def("is_button_down", &GamepadState::is_button_down)
        .def("__repr__", &GamepadState::to_string);
}
