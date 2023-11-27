#include "shader_object.h"

#include "kali/device/reflection.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"
#include "kali/device/helpers.h"
#include "kali/device/command.h"

namespace kali {

inline gfx::ShaderOffset gfx_shader_offset(const ShaderOffset& offset)
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

const TypeLayoutReflection* ShaderObject::element_type_layout() const
{
    return TypeLayoutReflection::from_slang(m_shader_object->getElementTypeLayout());
}

uint32_t ShaderObject::get_entry_point_count() const
{
    return m_shader_object->getEntryPointCount();
}

void ShaderObject::set_object(const ShaderOffset& offset, const ref<ShaderObject>& object)
{
    SLANG_CALL(m_shader_object->setObject(gfx_shader_offset(offset), object->gfx_shader_object()));
}

void ShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    SLANG_CALL(m_shader_object->setResource(gfx_shader_offset(offset), resource_view->gfx_resource_view()));
}

void ShaderObject::set_sampler(const ShaderOffset& offset, const ref<Sampler>& sampler)
{
    SLANG_CALL(m_shader_object->setSampler(gfx_shader_offset(offset), sampler->gfx_sampler_state()));
}

void ShaderObject::set_data(const ShaderOffset& offset, void const* data, size_t size)
{
    SLANG_CALL(m_shader_object->setData(gfx_shader_offset(offset), data, size));
}

//
// TransientShaderObject
//

TransientShaderObject::TransientShaderObject(gfx::IShaderObject* shader_object, CommandStream* stream)
    : ShaderObject(shader_object)
    , m_stream(stream)
{
}

ref<ShaderObject> TransientShaderObject::get_entry_point(uint32_t index)
{
    auto object = make_ref<TransientShaderObject>(m_shader_object->getEntryPoint(index), m_stream);
    m_sub_objects.push_back(object);
    return object;
}

ref<ShaderObject> TransientShaderObject::get_object(const ShaderOffset& offset)
{
    auto object = make_ref<TransientShaderObject>(m_shader_object->getObject(gfx_shader_offset(offset)), m_stream);
    m_sub_objects.push_back(object);
    return object;
}

void TransientShaderObject::set_object(const ShaderOffset& offset, const ref<ShaderObject>& object)
{
    if (ref<MutableShaderObject> mutable_object = dynamic_ref_cast<MutableShaderObject>(object)) {
        mutable_object->insert_barriers(m_stream);
    }

    ShaderObject::set_object(offset, object);
}

void TransientShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    switch (resource_view->type()) {
    case ResourceViewType::unknown:
        break;
    case ResourceViewType::render_target:
        m_stream->resource_barrier(resource_view->resource(), ResourceState::render_target);
        break;
    case ResourceViewType::depth_stencil:
        m_stream->resource_barrier(resource_view->resource(), ResourceState::render_target);
        break;
    case ResourceViewType::shader_resource:
        m_stream->resource_barrier(resource_view->resource(), ResourceState::shader_resource);
        break;
    case ResourceViewType::unordered_access:
        m_stream->resource_barrier(resource_view->resource(), ResourceState::unordered_access);
        break;
        // case ResourceViewType::acceleration_structure:
        //     m_stream->resource_barrier(resource_view->resource(), ResourceState::acceleration_structure);
        //     break;
    }

    ShaderObject::set_resource(offset, resource_view);
}

//
// MutableShaderObject
//

void MutableShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    ShaderObject::set_resource(offset, resource_view);

    if (resource_view)
        m_resource_views[offset] = resource_view;
    else
        m_resource_views.erase(offset);
}

void MutableShaderObject::insert_barriers(CommandStream* stream)
{
    for (auto& [offset, resource_view] : m_resource_views) {
        switch (resource_view->type()) {
        case ResourceViewType::unknown:
            break;
        case ResourceViewType::render_target:
            stream->resource_barrier(resource_view->resource(), ResourceState::render_target);
            break;
        case ResourceViewType::depth_stencil:
            stream->resource_barrier(resource_view->resource(), ResourceState::render_target);
            break;
        case ResourceViewType::shader_resource:
            stream->resource_barrier(resource_view->resource(), ResourceState::shader_resource);
            break;
        case ResourceViewType::unordered_access:
            stream->resource_barrier(resource_view->resource(), ResourceState::unordered_access);
            break;
            // case ResourceViewType::acceleration_structure:
            //     stream->resource_barrier(resource_view->resource(), ResourceState::acceleration_structure);
            //     break;
        }
    }
}

} // namespace kali
