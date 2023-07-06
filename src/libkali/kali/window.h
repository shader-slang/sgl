#pragma once

#include "kali/platform.h"
#include "kali/object.h"
#include "kali/input.h"

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

    WindowHandle get_window_handle() const;

    void resize(uint32_t width, uint32_t height);
    uint32_t get_width() const { return m_width; }
    uint32_t get_height() const { return m_height; }

    // void set_position(uint2 position);
    // uint2 get_position() const { return m_position; }

    void set_title(std::string title);
    const std::string& get_title() const { return m_title; }

    void set_icon(const std::filesystem::path& path);

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
    using DropFilesCallback = std::function<void(const std::span<const char*> /* files */)>;

    void set_on_resize(ResizeCallback on_resize) { m_on_resize = on_resize; }
    ResizeCallback get_on_resize() const { return m_on_resize; }
    void set_on_keyboard_event(KeyboardEventCallback on_keyboard_event) { m_on_keyboard_event = on_keyboard_event; }
    KeyboardEventCallback get_on_keyboard_event() const { return m_on_keyboard_event; }
    void set_on_mouse_event(MouseEventCallback on_mouse_event) { m_on_mouse_event = on_mouse_event; }
    MouseEventCallback get_on_mouse_event() const { return m_on_mouse_event; }
    void set_on_gamepad_event(GamepadEventCallback on_gamepad_event) { m_on_gamepad_event = on_gamepad_event; }
    GamepadEventCallback get_on_gamepad_event() const { return m_on_gamepad_event; }
    void set_on_gamepad_state(GamepadStateCallback on_gamepad_state) { m_on_gamepad_state = on_gamepad_state; }
    GamepadStateCallback get_on_gamepad_state() const { return m_on_gamepad_state; }
    void set_on_drop_files(DropFilesCallback on_drop_files) { m_on_drop_files = on_drop_files; }
    DropFilesCallback get_on_drop_files() const { return m_on_drop_files; }

private:
    void poll_gamepad_input();

    void handle_window_size(uint32_t width, uint32_t height);
    void handle_keyboard_event(const KeyboardEvent& event);
    void handle_mouse_event(const MouseEvent& event);
    void handle_gamepad_event(const GamepadEvent& event);
    void handle_drop_files(const std::span<const char*> files);

    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
    GLFWwindow* m_window;

    float2 m_mouse_pos{0.f, 0.f};
    KeyModifierFlags m_mods;

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
