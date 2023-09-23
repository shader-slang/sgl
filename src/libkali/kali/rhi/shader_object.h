#pragma once

#include "kali/core/object.h"
#include "kali/core/type_utils.h"

#include "kali/rhi/fwd.h"
#include "kali/rhi/shader_offset.h"

#include <slang-gfx.h>

#include <string_view>

namespace kali {

class ShaderObject : public Object {
    KALI_OBJECT(ShaderObject)
public:
    ShaderObject(gfx::IShaderObject* shader_object);

    virtual uint32_t get_entry_point_count() const;
    virtual ref<ShaderObject> get_entry_point(uint32_t index) = 0;

    virtual ref<ShaderObject> get_object(const ShaderOffset& offset) = 0;
    virtual void set_object(const ShaderOffset& offset, const ref<ShaderObject>& object);

    virtual void set_data(const ShaderOffset& offset, void const* data, size_t size);
    virtual void set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource);
    virtual void set_sampler(const ShaderOffset& offset, const ref<Sampler>& sampler);

    gfx::IShaderObject* get_gfx_shader_object() const { return m_shader_object; }

protected:
    gfx::IShaderObject* m_shader_object;
};

class TransientShaderObject : public ShaderObject {
    KALI_OBJECT(TransientShaderObject)
public:
    TransientShaderObject(gfx::IShaderObject* shader_object);

    virtual ref<ShaderObject> get_entry_point(uint32_t index) override;

    virtual ref<ShaderObject> get_object(const ShaderOffset& offset) override;
    virtual void set_object(const ShaderOffset& offset, const ref<ShaderObject>& object) override;

    virtual void set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource) override;
};

class MutableShaderObject : public ShaderObject {
    KALI_OBJECT(MutableShaderObject)
public:
    MutableShaderObject(gfx::IShaderObject* shader_object);
};

} // namespace kali
