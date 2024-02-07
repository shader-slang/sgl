#include "input.h"

#include "kali/core/format.h"

#include "kali/math/vector.h"

namespace kali {

std::string KeyboardEvent::to_string() const
{
    return fmt::format("KeyboardEvent(type={}, key={}, codepoint={:08x}, mods={})", type, key, codepoint, mods);
}

std::string MouseEvent::to_string() const
{
    return fmt::format("MouseEvent(type={}, pos={}, scroll={}, button={}, mods={})", type, pos, scroll, button, mods);
}

std::string GamepadEvent::to_string() const
{
    return fmt::format("GamepadEvent(type={}, button={})", type, button);
}

std::string GamepadState::to_string() const
{
    return fmt::format(
        "GamepadState(\n"
        "  left_x = {},\n"
        "  left_y = {},\n"
        "  right_x = {},\n"
        "  right_y = {},\n"
        "  left_trigger = {},\n"
        "  right_trigger = {},\n"
        "  buttons = {}\n"
        ")",
        left_x,
        left_y,
        right_x,
        right_y,
        left_trigger,
        right_trigger,
        buttons
    );
}

} // namespace kali
