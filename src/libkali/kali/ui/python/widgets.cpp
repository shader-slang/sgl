// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/ui/widgets.h"

namespace kali::ui {

template<typename T>
static void bind_value_property(nb::module_ m, const char* name)
{
    nb::class_<T, Widget>(m, name)
        .def_prop_rw("label", &T::label, &T::set_label)
        .def_prop_rw("value", &T::value, &T::set_value)
        .def_prop_rw("callback", &T::callback, &T::set_callback)
        .def("_get_callback", &T::callback);
}

template<typename T>
static void bind_drag(nb::module_ m, const char* name)
{
    nb::class_<T, ValueProperty<typename T::value_type>>(m, name)
        .def(
            nb::init<
                Widget*,
                std::string_view,
                typename T::value_type,
                typename T::Callback,
                float,
                typename T::scalar_type,
                typename T::scalar_type,
                std::string_view,
                SliderFlags>(),
            "parent"_a,
            "label"_a = "",
            "value"_a = typename T::value_type(0),
            "callback"_a = typename T::Callback{},
            "speed"_a = 1.f,
            "min"_a = typename T::scalar_type(0),
            "max"_a = typename T::scalar_type(0),
            "format"_a = T::default_format,
            "flags"_a = SliderFlags::none
        )
        .def_prop_rw("speed", &T::speed, &T::set_speed)
        .def_prop_rw("min", &T::min, &T::set_min)
        .def_prop_rw("max", &T::max, &T::set_max)
        .def_prop_rw("format", &T::format, &T::set_format)
        .def_prop_rw("flags", &T::flags, &T::set_flags);
}

template<typename T>
static void bind_slider(nb::module_ m, const char* name)
{
    nb::class_<T, ValueProperty<typename T::value_type>>(m, name)
        .def(
            nb::init<
                Widget*,
                std::string_view,
                typename T::value_type,
                typename T::Callback,
                typename T::scalar_type,
                typename T::scalar_type,
                std::string_view,
                SliderFlags>(),
            "parent"_a,
            "label"_a = "",
            "value"_a = typename T::value_type(0),
            "callback"_a = typename T::Callback{},
            "min"_a = typename T::scalar_type(0),
            "max"_a = typename T::scalar_type(0),
            "format"_a = T::default_format,
            "flags"_a = SliderFlags::none
        )
        .def_prop_rw("min", &T::min, &T::set_min)
        .def_prop_rw("max", &T::max, &T::set_max)
        .def_prop_rw("format", &T::format, &T::set_format)
        .def_prop_rw("flags", &T::flags, &T::set_flags);
}

} // namespace kali::ui

using Widget = kali::ui::Widget;

// GC traversal and clear functions for the window callbacks.
// This is used to clean up cyclic references which can easily occur with callbacks.
// See https://nanobind.readthedocs.io/en/latest/typeslots.html#reference-cycles-involving-functions
int widget_tp_traverse(PyObject* self, visitproc visit, void* arg)
{
    Widget* w = nb::inst_ptr<Widget>(self);

    // Visit nested children
    for (Widget* wc : w->children()) {
        PyObject* o = wc->self_py();
        Py_VISIT(o);
    }

    // Visit callback functions.
    // For now we rely on widgets to expose "_get_callback" methods that return the callback function.
    // It would be better to use the properties but I haven't figured out how to do that yet.
    nb::handle self_obj = nb::handle(self);

#define VISIT(callback)                                                                                                \
    {                                                                                                                  \
        if (nb::hasattr(self_obj, #callback)) {                                                                        \
            nb::object obj = nb::getattr(self_obj, "_get" #callback);                                                  \
            Py_VISIT(obj.ptr());                                                                                       \
        }                                                                                                              \
    }

    // VISIT(callback);

#undef VISIT

    return 0;
}

int widget_tp_clear(PyObject* self)
{
    Widget* w = nb::inst_ptr<Widget>(self);
    w->clear_children();
    return 0;
}

PyType_Slot widget_type_slots[] = {
    {Py_tp_traverse, (void*)widget_tp_traverse},
    {Py_tp_clear, (void*)widget_tp_clear},
    {0, nullptr},
};


KALI_PY_EXPORT(ui_widgets)
{
    using namespace kali;
    using namespace kali::ui;

    nb::module_ ui = m.attr("ui");

    nb::class_<Widget, kali::Object> widget(ui, "Widget", nb::type_slots(widget_type_slots));

    widget.def_prop_rw("parent", (Widget * (Widget::*)(void)) & Widget::parent, &Widget::set_parent)
        .def_prop_ro("children", &Widget::children)
        .def_prop_rw("visible", &Widget::visible, &Widget::set_visible)
        .def_prop_rw("enabled", &Widget::enabled, &Widget::set_enabled);

    nb::class_<Screen, Widget>(ui, "Screen").def("dispatch_events", &Screen::dispatch_events);

    nb::class_<Window, Widget>(ui, "Window")
        .def(
            nb::init<Widget*, std::string_view, float2, float2>(),
            "parent"_a,
            "title"_a = "",
            "position"_a = float2(10.f, 10.f),
            "size"_a = float2(400.f, 400.f)
        )
        .def("show", &Window::show)
        .def("close", &Window::close)
        .def_prop_rw("title", &Window::title, &Window::set_title)
        .def_prop_rw("position", &Window::position, &Window::set_position)
        .def_prop_rw("size", &Window::size, &Window::set_size);

    nb::class_<Group, Widget>(ui, "Group")
        .def(nb::init<Widget*, std::string_view>(), "parent"_a, "label"_a = "")
        .def_prop_rw("label", &Group::label, &Group::set_label);

    nb::class_<Text, Widget>(ui, "Text")
        .def(nb::init<Widget*, std::string_view>(), "parent"_a, "text"_a = "")
        .def_prop_rw("text", &Text::text, &Text::set_text);

    nb::class_<ProgressBar, Widget>(ui, "ProgressBar")
        .def(nb::init<Widget*, float>(), "parent"_a, "fraction"_a = 0.f)
        .def_prop_rw("fraction", &ProgressBar::fraction, &ProgressBar::set_fraction);

    nb::class_<Button, Widget>(ui, "Button")
        .def(
            nb::init<Widget*, std::string_view, Button::Callback>(),
            "parent"_a,
            "label"_a = "",
            "callback"_a = Button::Callback{}
        )
        .def_prop_rw("label", &Button::label, &Button::set_label)
        .def_prop_rw("callback", &Button::callback, &Button::set_callback)
        .def("_get_callback", &Button::callback);

    bind_value_property<ValueProperty<bool>>(ui, "ValuePropertyBool");
    bind_value_property<ValueProperty<int>>(ui, "ValuePropertyInt");
    bind_value_property<ValueProperty<int2>>(ui, "ValuePropertyInt2");
    bind_value_property<ValueProperty<int3>>(ui, "ValuePropertyInt3");
    bind_value_property<ValueProperty<int4>>(ui, "ValuePropertyInt4");
    bind_value_property<ValueProperty<float>>(ui, "ValuePropertyFloat");
    bind_value_property<ValueProperty<float2>>(ui, "ValuePropertyFloat2");
    bind_value_property<ValueProperty<float3>>(ui, "ValuePropertyFloat3");
    bind_value_property<ValueProperty<float4>>(ui, "ValuePropertyFloat4");
    bind_value_property<ValueProperty<std::string>>(ui, "ValuePropertyString");

    nb::class_<CheckBox, ValueProperty<bool>>(ui, "CheckBox")
        .def(
            nb::init<Widget*, std::string_view, bool, CheckBox::Callback>(),
            "parent"_a,
            "label"_a = "",
            "value"_a = false,
            "callback"_a = CheckBox::Callback{}
        );

    nb::class_<ComboBox, ValueProperty<int>>(ui, "ComboBox")
        .def(
            nb::init<Widget*, std::string_view, int, ComboBox::Callback, std::vector<std::string>>(),
            "parent"_a,
            "label"_a = "",
            "value"_a = 0,
            "callback"_a = ComboBox::Callback{},
            "items"_a = std::vector<std::string>{}
        )
        .def_prop_rw("items", &ComboBox::items, &ComboBox::set_items);

    nb::class_<ListBox, ValueProperty<int>>(ui, "ListBox")
        .def(
            nb::init<Widget*, std::string_view, int, ListBox::Callback, std::vector<std::string>, int>(),
            "parent"_a,
            "label"_a = "",
            "value"_a = 0,
            "callback"_a = ListBox::Callback{},
            "items"_a = std::vector<std::string>{},
            "height_in_items"_a = -1
        )
        .def_prop_rw("items", &ListBox::items, &ListBox::set_items)
        .def_prop_rw("height_in_items", &ListBox::height_in_items, &ListBox::set_height_in_items);

    nb::enum_<SliderFlags>(ui, "SliderFlags")
        .value("none", SliderFlags::none)
        .value("always_clamp", SliderFlags::always_clamp)
        .value("logarithmic", SliderFlags::logarithmic)
        .value("no_round_to_format", SliderFlags::no_round_to_format)
        .value("no_input", SliderFlags::no_input)
        .def_enum_operators();

    bind_drag<DragFloat>(ui, "DragFloat");
    bind_drag<DragFloat2>(ui, "DragFloat2");
    bind_drag<DragFloat3>(ui, "DragFloat3");
    bind_drag<DragFloat4>(ui, "DragFloat4");
    bind_drag<DragInt>(ui, "DragInt");
    bind_drag<DragInt2>(ui, "DragInt2");
    bind_drag<DragInt3>(ui, "DragInt3");
    bind_drag<DragInt4>(ui, "DragInt4");

    bind_slider<SliderFloat>(ui, "SliderFloat");
    bind_slider<SliderFloat2>(ui, "SliderFloat2");
    bind_slider<SliderFloat3>(ui, "SliderFloat3");
    bind_slider<SliderFloat4>(ui, "SliderFloat4");
    bind_slider<SliderInt>(ui, "SliderInt");
    bind_slider<SliderInt2>(ui, "SliderInt2");
    bind_slider<SliderInt3>(ui, "SliderInt3");
    bind_slider<SliderInt4>(ui, "SliderInt4");

    nb::enum_<InputTextFlags>(ui, "InputTextFlags")
        .value("none", InputTextFlags::none)
        .value("chars_decimal", InputTextFlags::chars_decimal)
        .value("chars_hexadecimal", InputTextFlags::chars_hexadecimal)
        .value("chars_uppercase", InputTextFlags::chars_uppercase)
        .value("chars_no_blank", InputTextFlags::chars_no_blank)
        .value("auto_select_all", InputTextFlags::auto_select_all)
        .value("enter_returns_true", InputTextFlags::enter_returns_true)
        .value("callback_completion", InputTextFlags::callback_completion)
        .value("callback_history", InputTextFlags::callback_history)
        .value("callback_always", InputTextFlags::callback_always)
        .value("callback_char_filter", InputTextFlags::callback_char_filter)
        .value("allow_tab_input", InputTextFlags::allow_tab_input)
        .value("ctrl_enter_for_new_line", InputTextFlags::ctrl_enter_for_new_line)
        .value("no_horizontal_scroll", InputTextFlags::no_horizontal_scroll)
        .value("always_overwrite", InputTextFlags::always_overwrite)
        .value("read_only", InputTextFlags::read_only)
        .value("password", InputTextFlags::password)
        .value("no_undo_redo", InputTextFlags::no_undo_redo)
        .value("chars_scientific", InputTextFlags::chars_scientific)
        .value("escape_clears_all", InputTextFlags::escape_clears_all)
        .def_enum_operators();

    nb::class_<InputText, ValueProperty<std::string>>(ui, "InputText")
        .def(
            nb::init<Widget*, std::string_view, std::string, InputText::Callback, bool, InputTextFlags>(),
            "parent"_a,
            "label"_a = "",
            "value"_a = false,
            "callback"_a = InputText::Callback{},
            "multi_line"_a = false,
            "flags"_a = InputTextFlags::none
        );
}
