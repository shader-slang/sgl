#pragma once

#include "kali/core/platform.h"
#include "kali/core/object.h"
#include "kali/core/input.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>

// GLFW forward declarations
struct GLFWwindow;

namespace kali {

enum class WindowMode {
    normal,
    minimized,
    fullscreen,
};

struct WindowDesc {
    uint32_t width;
    uint32_t height;
    std::string title;
    WindowMode mode{WindowMode::normal};
    bool resizable{true};
};

class KALI_API Window : public Object {
    KALI_OBJECT(Window)
public:
    Window(WindowDesc desc);
    ~Window();

    static ref<Window> create(WindowDesc desc) { return make_ref<Window>(desc); }

    WindowHandle window_handle() const;

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    void resize(uint32_t width, uint32_t height);

    // void set_position(uint2 position);
    // uint2 get_position() const { return m_position; }

    const std::string& title() const { return m_title; }
    void set_title(std::string title);

    void set_icon(const std::filesystem::path& path);

    void close();
    bool should_close() const;

    void process_events();

    void main_loop();

    // clipboard

    void set_clipboard(const std::string& text);
    std::optional<std::string> get_clipboard() const;

    // events

    using ResizeCallback = std::function<void(uint32_t /* width */, uint32_t /* height */)>;
    using KeyboardEventCallback = std::function<void(const KeyboardEvent& /* event */)>;
    using MouseEventCallback = std::function<void(const MouseEvent& /* event */)>;
    using GamepadEventCallback = std::function<void(const GamepadEvent& /* event */)>;
    using GamepadStateCallback = std::function<void(const GamepadState& /* state */)>;
    using DropFilesCallback = std::function<void(std::span<const char*> /* files */)>;

    const ResizeCallback& on_resize() const { return m_on_resize; }
    void set_on_resize(ResizeCallback on_resize) { m_on_resize = std::move(on_resize); }

    const KeyboardEventCallback& on_keyboard_event() const { return m_on_keyboard_event; }
    void set_on_keyboard_event(KeyboardEventCallback on_keyboard_event)
    {
        m_on_keyboard_event = std::move(on_keyboard_event);
    }

    const MouseEventCallback& on_mouse_event() const { return m_on_mouse_event; }
    void set_on_mouse_event(MouseEventCallback on_mouse_event) { m_on_mouse_event = std::move(on_mouse_event); }

    const GamepadEventCallback& on_gamepad_event() const { return m_on_gamepad_event; }
    void set_on_gamepad_event(GamepadEventCallback on_gamepad_event)
    {
        m_on_gamepad_event = std::move(on_gamepad_event);
    }

    const GamepadStateCallback& on_gamepad_state() const { return m_on_gamepad_state; }
    void set_on_gamepad_state(GamepadStateCallback on_gamepad_state)
    {
        m_on_gamepad_state = std::move(on_gamepad_state);
    }

    const DropFilesCallback& on_drop_files() const { return m_on_drop_files; }
    void set_on_drop_files(DropFilesCallback on_drop_files) { m_on_drop_files = std::move(on_drop_files); }

    std::string to_string() const override;

private:
    void poll_gamepad_input();

    void handle_window_size(uint32_t width, uint32_t height);
    void handle_keyboard_event(const KeyboardEvent& event);
    void handle_mouse_event(const MouseEvent& event);
    void handle_gamepad_event(const GamepadEvent& event);
    void handle_drop_files(std::span<const char*> files);

    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
    GLFWwindow* m_window;

    bool m_should_close{false};

    float2 m_mouse_pos{0.f, 0.f};
    KeyModifierFlags m_mods{KeyModifierFlags::none};

    static constexpr int INVALID_GAMEPAD_ID = -1;
    int m_gamepad_id{INVALID_GAMEPAD_ID};
    GamepadState m_gamepad_prev_state;

    ResizeCallback m_on_resize;
    KeyboardEventCallback m_on_keyboard_event;
    MouseEventCallback m_on_mouse_event;
    GamepadEventCallback m_on_gamepad_event;
    GamepadStateCallback m_on_gamepad_state;
    DropFilesCallback m_on_drop_files;

    friend struct EventHandlers;
};

} // namespace kali
