#pragma once

#include "kali/core/object.h"
#include "kali/math/vector_types.h"

#include <imgui.h>

#include <string_view>
#include <string>
#include <functional>
#include <vector>

namespace kali::ui {

class Widget;

struct Event {
    Widget* sender;
};

/// Base class for Python UI widgets.
/// Widgets own their children.
class Widget : public Object {
    KALI_OBJECT(Widget)
public:
    Widget(Widget* parent)
        : m_parent(parent)
    {
        if (m_parent)
            m_parent->m_children.push_back(ref<Widget>(this));
    }

    virtual ~Widget() { }

    Widget* parent() { return m_parent; }
    const Widget* parent() const { return m_parent; }
    void set_parent(Widget* parent) { m_parent = parent; }

    const std::vector<ref<Widget>>& children() const { return m_children; }
    void clear_children() { m_children.clear(); }

    bool visible() const { return m_visible; }
    void set_visible(bool visible) { m_visible = visible; }

    bool enabled() const { return m_enabled; }
    void set_enabled(bool enabled) { m_enabled = enabled; }

    virtual void render()
    {
        if (m_visible)
            for (const auto& child : m_children)
                child->render();
    }

    virtual void record_event(const Event& event)
    {
        if (m_parent)
            m_parent->record_event(event);
    }

    virtual void dispatch_event(const Event& event) { KALI_UNUSED(event); }

protected:
    Widget* m_parent;
    std::vector<ref<Widget>> m_children;
    bool m_visible{true};
    bool m_enabled{true};
};

/// This is the main widget that represents the screen.
/// It is intended to be used as the parent for \c Window widgets.
class Screen : public Widget {
    KALI_OBJECT(Screen)
public:
    Screen()
        : Widget(nullptr)
    {
    }

    virtual void render() override { Widget::render(); }

    virtual void record_event(const Event& event) override { m_events.push_back(event); }

    void dispatch_events()
    {
        for (const auto& event : m_events)
            event.sender->dispatch_event(event);
        m_events.clear();
    }

private:
    std::vector<Event> m_events;
};

/// Scoped push/pop of ImGui ID.
class ScopedID {
public:
    ScopedID(void* id) { ImGui::PushID(id); }
    ~ScopedID() { ImGui::PopID(); }
};

/// Scoped begin/end for disabling ImGUI widgets.
class ScopedDisable {
public:
    ScopedDisable(bool disabled)
        : m_disabled(disabled)
    {
        if (disabled)
            ImGui::BeginDisabled();
    }
    ~ScopedDisable()
    {
        if (m_disabled)
            ImGui::EndDisabled();
    }

private:
    bool m_disabled;
};

class Window : public Widget {
    KALI_OBJECT(Window)
public:
    Window(
        Widget* parent,
        std::string_view title = "",
        float2 position = float2(10.f, 10.f),
        float2 size = float2(400.f, 400.f)
    )
        : Widget(parent)
        , m_title(title)
        , m_position(position)
        , m_size(size)
    {
    }

    const std::string& title() const { return m_title; }
    void set_title(std::string_view title) { m_title = title; }

    float2 position() const { return m_position; }
    void set_position(const float2& position)
    {
        m_position = position;
        m_set_position = true;
    }

    float2 size() const { return m_size; }
    void set_size(const float2& size)
    {
        m_size = size;
        m_set_size = true;
    }

    void show() { set_visible(true); }
    void close() { set_visible(false); }

    virtual void render() override
    {
        if (!m_visible)
            return;

        if (m_set_position) {
            ImGui::SetNextWindowPos(ImVec2(m_position.x, m_position.y));
            m_set_position = false;
        }
        if (m_set_size) {
            ImGui::SetNextWindowSize(ImVec2(m_size.x, m_size.y));
            m_set_size = false;
        }

        ScopedID id(this);
        if (ImGui::Begin(m_title.c_str(), &m_visible)) {
            auto pos = ImGui::GetWindowPos();
            m_position = float2(pos.x, pos.y);
            auto size = ImGui::GetWindowSize();
            m_size = float2(size.x, size.y);

            ImGui::PushItemWidth(300);
            Widget::render();
            ImGui::PopItemWidth();
        }
        ImGui::End();
    }

private:
    std::string m_title;
    float2 m_position;
    float2 m_size;
    bool m_set_position{true};
    bool m_set_size{true};
};

class Group : public Widget {
    KALI_OBJECT(Group)
public:
    Group(Widget* parent, std::string_view label = "")
        : Widget(parent)
        , m_label(label)
    {
    }

    const std::string& label() const { return m_label; }
    void set_label(std::string_view label) { m_label = label; }

    virtual void render() override
    {
        // Check if this is a nested group
        bool nested = false;
        for (Widget* p = parent(); p != nullptr; p = p->parent())
            if (dynamic_cast<Group*>(p) != nullptr)
                nested = true;

        ScopedID id(this);
        ScopedDisable disable(!m_enabled);

        if (nested ? ImGui::TreeNodeEx(m_label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)
                   : ImGui::CollapsingHeader(m_label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            Widget::render();
            if (nested)
                ImGui::TreePop();
        }
    }

private:
    std::string m_label;
};

class Text : public Widget {
    KALI_OBJECT(Text)
public:
    Text(Widget* parent, std::string_view text = "")
        : Widget(parent)
        , m_text(text)
    {
    }

    const std::string& text() const { return m_text; }
    void set_text(std::string_view text) { m_text = text; }

    virtual void render() override
    {
        ScopedID id(this);
        ScopedDisable disable(!m_enabled);
        ImGui::TextUnformatted(m_text.c_str());
    }

private:
    std::string m_text;
};

class ProgressBar : public Widget {
    KALI_OBJECT(ProgressBar)
public:
    ProgressBar(Widget* parent, float fraction = 0.f)
        : Widget(parent)
        , m_fraction(fraction)
    {
    }

    float fraction() const { return m_fraction; }
    void set_fraction(float fraction) { m_fraction = fraction; }

    virtual void render() override
    {
        ScopedID id(this);
        ScopedDisable disable(!m_enabled);
        ImGui::ProgressBar(m_fraction);
    }

private:
    float m_fraction;
};

class Button : public Widget {
    KALI_OBJECT(Button)
public:
    using Callback = std::function<void()>;

    Button(Widget* parent, std::string_view label = "", Callback callback = {})
        : Widget(parent)
        , m_label(label)
        , m_callback(callback)
    {
    }

    const std::string& label() const { return m_label; }
    void set_label(std::string_view label) { m_label = label; }

    Callback callback() const { return m_callback; }
    void set_callback(Callback callback) { m_callback = callback; }

    virtual void render() override
    {
        ScopedID id(this);
        ScopedDisable disable(!m_enabled);
        if (ImGui::Button(m_label.c_str()))
            record_event({this});
    }

    virtual void dispatch_event(const Event& event) override
    {
        KALI_ASSERT(event.sender == this);
        if (m_callback)
            m_callback();
    }

private:
    std::string m_label;
    Callback m_callback;
};

template<typename T>
class ValueProperty : public Widget {
public:
    using value_type = T;
    using Callback = std::function<void(const value_type&)>;

    ValueProperty(Widget* parent, std::string_view label, Callback callback, const value_type& value)
        : Widget(parent)
        , m_label(label)
        , m_callback(callback)
        , m_value(value)
    {
    }

    const std::string& label() const { return m_label; }
    void set_label(std::string_view label) { m_label = label; }

    Callback callback() const { return m_callback; }
    void set_callback(Callback callback) { m_callback = callback; }

    value_type value() const { return m_value; }
    void set_value(value_type value) { m_value = value; }

    virtual void dispatch_event(const Event& event) override
    {
        KALI_ASSERT(event.sender == this);
        if (m_callback)
            m_callback(m_value);
    }

protected:
    std::string m_label;
    Callback m_callback;
    value_type m_value;
};

class Checkbox : public ValueProperty<bool> {
    KALI_OBJECT(Checkbox)
public:
    using Base = ValueProperty<bool>;

    Checkbox(Widget* parent, std::string_view label = "", Callback callback = {}, bool value = false)
        : Base(parent, label, callback, value)
    {
    }

    virtual void render() override
    {
        ScopedID id(this);
        ScopedDisable disable(!m_enabled);
        if (ImGui::Checkbox(m_label.c_str(), &m_value))
            record_event({this});
    }
};

class Combobox : public ValueProperty<int> {
    KALI_OBJECT(Combobox)
public:
    using Base = ValueProperty<int>;

    Combobox(
        Widget* parent,
        std::string_view label = "",
        Callback callback = {},
        std::vector<std::string> items = {},
        int value = 0
    )
        : Base(parent, label, callback, value)
        , m_items(items)
    {
    }

    const std::vector<std::string>& items() const { return m_items; }
    void set_items(const std::vector<std::string>& items) { m_items = items; }

    virtual void render() override
    {
        ScopedID id(this);
        ScopedDisable disable(!m_enabled);
        if (ImGui::Combo(
                m_label.c_str(),
                &m_value,
                [](void* data, int idx, const char** out_text) -> bool
                {
                    auto& items = *reinterpret_cast<std::vector<std::string>*>(data);
                    if (idx < 0 || idx >= items.size())
                        return false;
                    *out_text = items[idx].c_str();
                    return true;
                },
                &m_items,
                (int)m_items.size()
            )) {
            record_event({this});
        }
    }

private:
    std::vector<std::string> m_items;
};

enum class SliderFlags {
    none = ImGuiSliderFlags_None,
    always_clamp = ImGuiSliderFlags_AlwaysClamp,
    logarithmic = ImGuiSliderFlags_Logarithmic,
    no_round_to_format = ImGuiSliderFlags_NoRoundToFormat,
    no_input = ImGuiSliderFlags_NoInput,
};

template<typename T>
struct SliderTraits { };

// clang-format off
template<> struct SliderTraits<float> { using type = float; using scalar_type = float; static constexpr bool is_float = true; static constexpr int N = 1; static constexpr const char* default_format = "%.3f"; };
template<> struct SliderTraits<float2> { using type = float2; using scalar_type = float; static constexpr bool is_float = true; static constexpr int N = 2; static constexpr const char* default_format = "%.3f"; };
template<> struct SliderTraits<float3> { using type = float3; using scalar_type = float; static constexpr bool is_float = true; static constexpr int N = 3; static constexpr const char* default_format = "%.3f"; };
template<> struct SliderTraits<float4> { using type = float4; using scalar_type = float; static constexpr bool is_float = true; static constexpr int N = 4; static constexpr const char* default_format = "%.3f"; };
template<> struct SliderTraits<int> { using type = int; using scalar_type = int; static constexpr bool is_float = false; static constexpr int N = 1; static constexpr const char* default_format = "%d"; };
template<> struct SliderTraits<int2> { using type = int2; using scalar_type = int; static constexpr bool is_float = false; static constexpr int N = 2; static constexpr const char* default_format = "%d"; };
template<> struct SliderTraits<int3> { using type = int3; using scalar_type = int; static constexpr bool is_float = false; static constexpr int N = 3; static constexpr const char* default_format = "%d"; };
template<> struct SliderTraits<int4> { using type = int4; using scalar_type = int; static constexpr bool is_float = false; static constexpr int N = 4; static constexpr const char* default_format = "%d"; };
// clang-format on

template<typename T>
class Drag : public ValueProperty<T> {
    KALI_OBJECT(Drag)
public:
    using Base = ValueProperty<T>;

    using Widget::record_event;
    using Widget::m_enabled;
    using Base::m_label;
    using Base::m_value;

    using traits = SliderTraits<T>;
    using type = typename traits::type;
    using scalar_type = typename traits::scalar_type;
    static constexpr bool is_float = traits::is_float;
    static constexpr int N = traits::N;
    static constexpr const char* default_format = traits::default_format;

    Drag(
        Widget* parent,
        std::string_view label = "",
        typename Base::Callback callback = {},
        type value = type(0),
        float speed = 1.f,
        scalar_type min = scalar_type(0),
        scalar_type max = scalar_type(0),
        std::string_view format = default_format,
        SliderFlags flags = SliderFlags::none
    )
        : Base(parent, label, callback, value)
        , m_speed(speed)
        , m_min(min)
        , m_max(max)
        , m_format(format)
        , m_flags(flags)
    {
    }

    scalar_type speed() const { return static_cast<scalar_type>(m_speed); }
    void set_speed(scalar_type speed) { m_speed = static_cast<float>(speed); }

    scalar_type min() const { return m_min; }
    void set_min(scalar_type min) { m_min = min; }

    scalar_type max() const { return m_max; }
    void set_max(scalar_type max) { m_max = max; }

    const std::string& format() const { return m_format; }
    void set_format(std::string_view format) { m_format = format; }

    SliderFlags flags() const { return m_flags; }
    void set_flags(SliderFlags flags) { m_flags = flags; }

    virtual void render() override
    {
        ScopedID id(this);
        ScopedDisable disable(m_enabled);
        bool changed = false;
        if constexpr (is_float == true) {
            if constexpr (N == 1)
                changed = ImGui::DragFloat(
                    m_label.c_str(),
                    &m_value,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 2)
                changed = ImGui::DragFloat2(
                    m_label.c_str(),
                    &m_value.x,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 3)
                changed = ImGui::DragFloat3(
                    m_label.c_str(),
                    &m_value.x,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 4)
                changed = ImGui::DragFloat4(
                    m_label.c_str(),
                    &m_value.x,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
        } else {
            if constexpr (N == 1)
                changed = ImGui::DragInt(
                    m_label.c_str(),
                    &m_value,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 2)
                changed = ImGui::DragInt2(
                    m_label.c_str(),
                    &m_value.x,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 3)
                changed = ImGui::DragInt3(
                    m_label.c_str(),
                    &m_value.x,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 4)
                changed = ImGui::DragInt4(
                    m_label.c_str(),
                    &m_value.x,
                    m_speed,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
        }

        if (changed)
            record_event({this});
    }

private:
    float m_speed;
    scalar_type m_min;
    scalar_type m_max;
    std::string m_format;
    SliderFlags m_flags;
};

using DragFloat = Drag<float>;
using DragFloat2 = Drag<float2>;
using DragFloat3 = Drag<float3>;
using DragFloat4 = Drag<float4>;
using DragInt = Drag<int>;
using DragInt2 = Drag<int2>;
using DragInt3 = Drag<int3>;
using DragInt4 = Drag<int4>;

template<typename T>
class Slider : public ValueProperty<T> {
    KALI_OBJECT(Slider)
public:
    using Base = ValueProperty<T>;

    using Widget::record_event;
    using Widget::m_enabled;
    using Base::m_label;
    using Base::m_value;

    using traits = SliderTraits<T>;
    using type = typename traits::type;
    using scalar_type = typename traits::scalar_type;
    static constexpr bool is_float = traits::is_float;
    static constexpr int N = traits::N;
    static constexpr const char* default_format = traits::default_format;

    Slider(
        Widget* parent,
        std::string_view label = "",
        typename Base::Callback callback = {},
        type value = type(0),
        scalar_type min = scalar_type(0),
        scalar_type max = scalar_type(0),
        std::string_view format = default_format,
        SliderFlags flags = SliderFlags::none
    )
        : Base(parent, label, callback, value)
        , m_min(min)
        , m_max(max)
        , m_format(format)
        , m_flags(flags)
    {
    }

    scalar_type min() const { return m_min; }
    void set_min(scalar_type min) { m_min = min; }

    scalar_type max() const { return m_max; }
    void set_max(scalar_type max) { m_max = max; }

    const std::string& format() const { return m_format; }
    void set_format(std::string_view format) { m_format = format; }

    SliderFlags flags() const { return m_flags; }
    void set_flags(SliderFlags flags) { m_flags = flags; }

    virtual void render() override
    {
        ScopedID id(this);
        ScopedDisable disable(!m_enabled);
        bool changed = false;
        if constexpr (is_float == true) {
            if constexpr (N == 1)
                changed = ImGui::SliderFloat(
                    m_label.c_str(),
                    &m_value,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 2)
                changed = ImGui::SliderFloat2(
                    m_label.c_str(),
                    &m_value.x,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 3)
                changed = ImGui::SliderFloat3(
                    m_label.c_str(),
                    &m_value.x,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 4)
                changed = ImGui::SliderFloat4(
                    m_label.c_str(),
                    &m_value.x,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
        } else {
            if constexpr (N == 1)
                changed = ImGui::SliderInt(
                    m_label.c_str(),
                    &m_value,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 2)
                changed = ImGui::SliderInt2(
                    m_label.c_str(),
                    &m_value.x,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 3)
                changed = ImGui::SliderInt3(
                    m_label.c_str(),
                    &m_value.x,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
            if constexpr (N == 4)
                changed = ImGui::SliderInt4(
                    m_label.c_str(),
                    &m_value.x,
                    m_min,
                    m_max,
                    m_format.c_str(),
                    (ImGuiSliderFlags)m_flags
                );
        }

        if (changed)
            record_event({this});
    }

private:
    scalar_type m_min;
    scalar_type m_max;
    std::string m_format;
    SliderFlags m_flags;
};

using SliderFloat = Slider<float>;
using SliderFloat2 = Slider<float2>;
using SliderFloat3 = Slider<float3>;
using SliderFloat4 = Slider<float4>;
using SliderInt = Slider<int>;
using SliderInt2 = Slider<int2>;
using SliderInt3 = Slider<int3>;
using SliderInt4 = Slider<int4>;

} // namespace kali::ui