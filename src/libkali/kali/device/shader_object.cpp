#include "shader_object.h"

#include "kali/device/resource.h"
#include "kali/device/sampler.h"
#include "kali/device/helpers.h"

namespace kali {

inline gfx::ShaderOffset get_gfx_shader_offset(const ShaderOffset& offset)
{
    return {
        .uniformOffset = offset.uniform_offset,
        .bindingRangeIndex = narrow_cast<gfx::GfxIndex>(offset.binding_range_index),
        .bindingArrayIndex = narrow_cast<gfx::GfxIndex>(offset.binding_array_index),
    };
}


//
// ShaderObject
//

ShaderObject::ShaderObject(gfx::IShaderObject* shader_object)
    : m_shader_object(shader_object)
{
}

uint32_t ShaderObject::get_entry_point_count() const
{
    return m_shader_object->getEntryPointCount();
}

void ShaderObject::set_object(const ShaderOffset& offset, const ref<ShaderObject>& object)
{
    SLANG_CALL(m_shader_object->setObject(get_gfx_shader_offset(offset), object->get_gfx_shader_object()));
}

void ShaderObject::set_data(const ShaderOffset& offset, void const* data, size_t size)
{
    SLANG_CALL(m_shader_object->setData(get_gfx_shader_offset(offset), data, size));
}

void ShaderObject::set_sampler(const ShaderOffset& offset, const ref<Sampler>& sampler)
{
    SLANG_CALL(m_shader_object->setSampler(get_gfx_shader_offset(offset), sampler->get_gfx_sampler_state()));
}

void ShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource)
{
    SLANG_CALL(m_shader_object->setResource(get_gfx_shader_offset(offset), resource->get_gfx_resource_view()));
}

//
// TransientShaderObject
//

TransientShaderObject::TransientShaderObject(gfx::IShaderObject* shader_object)
    : ShaderObject(shader_object)
{
}

ref<ShaderObject> TransientShaderObject::get_entry_point(uint32_t index)
{
    return make_ref<TransientShaderObject>(m_shader_object->getEntryPoint(index));
}

ref<ShaderObject> TransientShaderObject::get_object(const ShaderOffset& offset)
{
    return make_ref<TransientShaderObject>(m_shader_object->getObject(get_gfx_shader_offset(offset)));
}

void TransientShaderObject::set_object(const ShaderOffset& offset, const ref<ShaderObject>& object)
{
    // TODO add barriers for object
    ShaderObject::set_object(offset, object);
}

void TransientShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource)
{
    ShaderObject::set_resource(offset, resource);
    // TODO add barrier
}

//
// MutableShaderObject
//


} // namespace kali
