#pragma once

#include <slang-gfx.h>

namespace kali {

class ShaderObject {
public:
    ShaderObject(gfx::IShaderObject* shader_object)
        : m_shader_object(shader_object)
    {
    }

    gfx::IShaderObject* get_gfx_shader_object() const { return m_shader_object; }

private:
    gfx::IShaderObject* m_shader_object;
};

} // namespace kali
