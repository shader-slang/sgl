#pragma once

#include "macros.h"
#include "enum.h"
#include "math/vector_types.h"

namespace kali {

enum class MouseButton : uint32_t {
    left,
    middle,
    right,
    unknown,
};

enum class KeyModifierFlags : uint32_t {
    none = 0,
    shift = 1,
    ctrl = 2,
    alt = 4,
};
KALI_ENUM_CLASS_OPERATORS(KeyModifierFlags);

enum class KeyModifier : uint32_t {
    shift = (uint32_t)KeyModifierFlags::shift,
    ctrl = (uint32_t)KeyModifierFlags::ctrl,
    alt = (uint32_t)KeyModifierFlags::alt
};

enum class KeyCode : uint32_t {
    // Key codes 0..255 are reserved for ASCII codes.
    space = ' ',
    apostrophe = '\'',
    comma = ',',
    minus = '-',
    period = '.',
    slash = '/',
    key0 = '0',
    key1 = '1',
    key2 = '2',
    key3 = '3',
    key4 = '4',
    key5 = '5',
    key6 = '6',
    key7 = '7',
    key8 = '8',
    key9 = '9',
    semicolon = ';',
    equal = '=',
    a = 'A',
    b = 'B',
    c = 'C',
    d = 'D',
    e = 'E',
    f = 'F',
    g = 'G',
    h = 'H',
    i = 'I',
    j = 'J',
    k = 'K',
    l = 'L',
    m = 'M',
    n = 'N',
    o = 'O',
    p = 'P',
    q = 'Q',
    r = 'R',
    s = 'S',
    t = 'T',
    u = 'U',
    v = 'V',
    w = 'W',
    x = 'X',
    y = 'Y',
    z = 'Z',
    left_bracket = '[',
    backslash = '\\',
    right_bracket = ']',
    grave_accent = '`',

    // Special keys start at key code 256.
    escape = 256,
    tab,
    enter,
    backspace,
    insert,
    del,
    right,
    left,
    down,
    up,
    page_up,
    page_down,
    home,
    end,
    caps_lock,
    scroll_lock,
    num_lock,
    print_screen,
    pause,
    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
    f10,
    f11,
    f12,
    keypad0,
    keypad1,
    keypad2,
    keypad3,
    keypad4,
    keypad5,
    keypad6,
    keypad7,
    keypad8,
    keypad9,
    keypad_del,
    keypad_divide,
    keypad_multiply,
    keypad_subtract,
    keypad_add,
    keypad_enter,
    keypad_equal,
    left_shift,
    left_control,
    left_alt,
    left_super,
    right_shift,
    right_control,
    right_alt,
    right_super,
    menu,
    unknown,
};

enum class KeyboardEventType {
    key_press,   ///< Key was pressed.
    key_release, ///< Key was released.
    key_repeat,  ///< Key is repeatedly down.
    input        ///< Character input.
};

struct KeyboardEvent {

    KeyboardEventType type;        ///< The event type.
    KeyCode key{KeyCode::unknown}; ///< The last key that was pressed/released.
    KeyModifierFlags mods{0};      ///< Keyboard modifier flags.
    uint32_t codepoint{0};         ///< UTF-32 codepoint from GLFW for events.

    bool is_key_press() const { return type == KeyboardEventType::key_press; }
    bool is_key_release() const { return type == KeyboardEventType::key_release; }
    bool is_key_repeat() const { return type == KeyboardEventType::key_repeat; }
    bool is_input() const { return type == KeyboardEventType::input; }

    bool has_modifier(KeyModifier mod) const { return is_set(mods, (KeyModifierFlags)mod); }
};

enum class MouseEventType {
    button_down, ///< Mouse button was pressed.
    button_up,   ///< Mouse button was released.
    move,        ///< Mouse cursor position changed.
    scroll       ///< Mouse wheel was scrolled.
};

struct MouseEvent {

    MouseEventType type;
    float2 pos{0.f, 0.f};
    float2 scroll{0.f, 0.f};
    MouseButton button{MouseButton::unknown};
    KeyModifierFlags mods{0};

    bool is_button_down() const { return type == MouseEventType::button_down; }
    bool is_button_up() const { return type == MouseEventType::button_up; }
    bool is_move() const { return type == MouseEventType::move; }
    bool is_scroll() const { return type == MouseEventType::scroll; }

    bool has_modifier(KeyModifier mod) const { return is_set(mods, (KeyModifierFlags)mod); }
};

enum class GamepadButton : uint32_t {
    a,
    b,
    x,
    y,
    left_bumper,
    right_bumper,
    back,
    start,
    guide,
    left_thumb,
    right_thumb,
    up,
    right,
    down,
    left,
};

struct GamepadEvent {
    enum class Type {
        button_down,
        button_up,
        connect,
        disconnect,
    };

    Type type;
    GamepadButton button;
};

struct GamepadState {
    float left_x;
    float left_y;
    float right_x;
    float right_y;
    float left_trigger;
    float right_trigger;

    uint32_t buttons;

    bool is_button_down(GamepadButton button) const { return buttons & (1 << uint32_t(button)); }
};

} // namespace kali
