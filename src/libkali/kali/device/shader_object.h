#pragma once

#include "kali/core/object.h"
#include "kali/core/type_utils.h"

#include "kali/device/fwd.h"
#include "kali/device/shader_offset.h"
#include "kali/device/resource.h"

#include <slang-gfx.h>

#include <string_view>
#include <vector>
#include <map>

namespace kali {

class KALI_API ShaderObject : public Object {
    KALI_OBJECT(ShaderObject)
public:
    ShaderObject(gfx::IShaderObject* shader_object);

    virtual const TypeLayoutReflection* element_type_layout() const;

    virtual uint32_t get_entry_point_count() const;
    virtual ref<ShaderObject> get_entry_point(uint32_t index) = 0;

    virtual ref<ShaderObject> get_object(const ShaderOffset& offset) = 0;
    virtual void set_object(const ShaderOffset& offset, const ref<ShaderObject>& object);

    virtual void set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view);
    virtual void set_sampler(const ShaderOffset& offset, const ref<Sampler>& sampler);
    virtual void set_acceleration_structure(const ShaderOffset& offset, const ref<AccelerationStructure>& acceleration_structure);
    virtual void set_data(const ShaderOffset& offset, void const* data, size_t size);

    gfx::IShaderObject* gfx_shader_object() const { return m_shader_object; }

protected:
    gfx::IShaderObject* m_shader_object;
};

class KALI_API TransientShaderObject : public ShaderObject {
    KALI_OBJECT(TransientShaderObject)
public:
    TransientShaderObject(gfx::IShaderObject* shader_object, CommandStream* command_stream);

    virtual ref<ShaderObject> get_entry_point(uint32_t index) override;

    virtual ref<ShaderObject> get_object(const ShaderOffset& offset) override;
    virtual void set_object(const ShaderOffset& offset, const ref<ShaderObject>& object) override;

    virtual void set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view) override;

private:
    CommandStream* m_command_stream;
    std::vector<ref<TransientShaderObject>> m_sub_objects;
};

class KALI_API MutableShaderObject : public ShaderObject {
    KALI_OBJECT(MutableShaderObject)
public:
    MutableShaderObject(gfx::IShaderObject* shader_object);

    virtual void set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view) override;

    void set_resource_states(CommandStream* command_stream);

private:
    std::map<ShaderOffset, ref<ResourceView>> m_resource_views;
};

} // namespace kali
