// SPDX-License-Identifier: Apache-2.0

#include "shader_object.h"

#include "sgl/device/reflection.h"
#include "sgl/device/resource.h"
#include "sgl/device/sampler.h"
#include "sgl/device/helpers.h"
#include "sgl/device/command.h"
#include "sgl/device/shader.h"
#include "sgl/device/device.h"
#include "sgl/device/cuda_interop.h"

namespace sgl {

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

ShaderObject::ShaderObject(ref<Device> device, gfx::IShaderObject* shader_object)
    : m_device(std::move(device))
    , m_shader_object(shader_object)
{
}

ref<const TypeLayoutReflection> ShaderObject::element_type_layout() const
{
    return TypeLayoutReflection::from_slang(ref(this), m_shader_object->getElementTypeLayout());
}

uint32_t ShaderObject::get_entry_point_count() const
{
    return m_shader_object->getEntryPointCount();
}

void ShaderObject::set_object(const ShaderOffset& offset, const ref<ShaderObject>& object)
{
    SLANG_CALL(m_shader_object->setObject(gfx_shader_offset(offset), object ? object->gfx_shader_object() : nullptr));
}

void ShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    SLANG_CALL(m_shader_object->setResource(
        gfx_shader_offset(offset),
        resource_view ? resource_view->gfx_resource_view() : nullptr
    ));
}

void ShaderObject::set_sampler(const ShaderOffset& offset, const ref<Sampler>& sampler)
{
    SLANG_CALL(m_shader_object->setSampler(gfx_shader_offset(offset), sampler ? sampler->gfx_sampler_state() : nullptr)
    );
}

void ShaderObject::set_acceleration_structure(
    const ShaderOffset& offset,
    const ref<AccelerationStructure>& acceleration_structure
)
{
    SLANG_CALL(m_shader_object->setResource(
        gfx_shader_offset(offset),
        acceleration_structure ? acceleration_structure->gfx_acceleration_structure() : nullptr
    ));
}

void ShaderObject::set_data(const ShaderOffset& offset, const void* data, size_t size)
{
    SLANG_CALL(m_shader_object->setData(gfx_shader_offset(offset), data, size));
}

void ShaderObject::set_cuda_tensor_view(const ShaderOffset& offset, const cuda::TensorView& tensor_view, bool is_uav)
{
    SGL_CHECK(m_device->supports_cuda_interop(), "Device does not support CUDA interop");
    ref<cuda::InteropBuffer> cuda_interop_buffer = make_ref<cuda::InteropBuffer>(m_device, tensor_view, is_uav);
    set_resource(offset, cuda_interop_buffer->get_resource_view());
    m_cuda_interop_buffers.push_back(cuda_interop_buffer);
}

void ShaderObject::get_cuda_interop_buffers(std::vector<ref<cuda::InteropBuffer>>& cuda_interop_buffers) const
{
    cuda_interop_buffers
        .insert(cuda_interop_buffers.end(), m_cuda_interop_buffers.begin(), m_cuda_interop_buffers.end());
}

//
// TransientShaderObject
//

TransientShaderObject::TransientShaderObject(
    ref<Device> device,
    gfx::IShaderObject* shader_object,
    CommandBuffer* command_buffer
)
    : ShaderObject(std::move(device), shader_object)
    , m_command_buffer(command_buffer)
{
}

ref<ShaderObject> TransientShaderObject::get_entry_point(uint32_t index)
{
    auto object = make_ref<TransientShaderObject>(m_device, m_shader_object->getEntryPoint(index), m_command_buffer);
    m_sub_objects.push_back(object);
    return object;
}

ref<ShaderObject> TransientShaderObject::get_object(const ShaderOffset& offset)
{
    auto object = make_ref<TransientShaderObject>(
        m_device,
        m_shader_object->getObject(gfx_shader_offset(offset)),
        m_command_buffer
    );
    m_sub_objects.push_back(object);
    return object;
}

void TransientShaderObject::set_object(const ShaderOffset& offset, const ref<ShaderObject>& object)
{
    if (ref<MutableShaderObject> mutable_object = dynamic_ref_cast<MutableShaderObject>(object)) {
        mutable_object->set_resource_states(m_command_buffer);
    }

    ShaderObject::set_object(offset, object);
}

void TransientShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    if (resource_view) {
        switch (resource_view->type()) {
        case ResourceViewType::shader_resource:
            m_command_buffer->set_resource_state(resource_view, ResourceState::shader_resource);
            break;
        case ResourceViewType::unordered_access:
            if (resource_view->resource()->state_tracker().global_state() == ResourceState::unordered_access)
                m_command_buffer->uav_barrier(resource_view->resource());
            else
                m_command_buffer->set_resource_state(resource_view, ResourceState::unordered_access);
            break;
        default:
            SGL_THROW("Invalid resource view type");
        }
    }

    ShaderObject::set_resource(offset, resource_view);
}

void TransientShaderObject::get_cuda_interop_buffers(std::vector<ref<cuda::InteropBuffer>>& cuda_interop_buffers) const
{
    ShaderObject::get_cuda_interop_buffers(cuda_interop_buffers);
    for (const auto& sub_object : m_sub_objects)
        sub_object->get_cuda_interop_buffers(cuda_interop_buffers);
}

//
// MutableShaderObject
//

MutableShaderObject::MutableShaderObject(ref<Device> device, gfx::IShaderObject* shader_object)
    : ShaderObject(std::move(device), shader_object)
{
    m_shader_object->addRef();
}

MutableShaderObject::MutableShaderObject(ref<Device> device, const ShaderProgram* shader_program)
    : ShaderObject(std::move(device), nullptr)
{
    m_device->gfx_device()->createMutableRootShaderObject(shader_program->gfx_shader_program(), &m_shader_object);
}

MutableShaderObject::MutableShaderObject(ref<Device> device, const TypeLayoutReflection* type_layout)
    : ShaderObject(std::move(device), nullptr)
{
    m_device->gfx_device()->createMutableShaderObjectFromTypeLayout(
        type_layout->get_slang_type_layout(),
        &m_shader_object
    );
}

MutableShaderObject::~MutableShaderObject()
{
    m_shader_object->release();
}

ref<ShaderObject> MutableShaderObject::get_entry_point(uint32_t index)
{
    SGL_UNUSED(index);
    return nullptr;
}

ref<ShaderObject> MutableShaderObject::get_object(const ShaderOffset& offset)
{
    auto it = m_sub_objects.find(offset);
    if (it != m_sub_objects.end())
        return it->second;
    auto object = make_ref<MutableShaderObject>(m_device, m_shader_object->getObject(gfx_shader_offset(offset)));
    m_sub_objects.insert({offset, object});
    return object;
}

void MutableShaderObject::set_object(const ShaderOffset& offset, const ref<ShaderObject>& object)
{
    if (ref<MutableShaderObject> mutable_object = dynamic_ref_cast<MutableShaderObject>(object)) {
        m_sub_objects.insert({offset, mutable_object});
    }

    ShaderObject::set_object(offset, object);
}

void MutableShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    ShaderObject::set_resource(offset, resource_view);

    if (resource_view)
        m_resource_views[offset] = resource_view;
    else
        m_resource_views.erase(offset);
}

void MutableShaderObject::set_resource_states(CommandBuffer* command_buffer) const
{
    for (auto& [offset, resource_view] : m_resource_views) {
        switch (resource_view->type()) {
        case ResourceViewType::shader_resource:
            command_buffer->set_resource_state(resource_view, ResourceState::shader_resource);
            break;
        case ResourceViewType::unordered_access:
            if (resource_view->resource()->state_tracker().global_state() == ResourceState::unordered_access)
                command_buffer->uav_barrier(resource_view->resource());
            else
                command_buffer->set_resource_state(resource_view, ResourceState::unordered_access);
            break;
        default:
            SGL_THROW("Invalid resource view type");
        }
    }
    for (auto& [_, sub_object] : m_sub_objects) {
        sub_object->set_resource_states(command_buffer);
    }
}

void MutableShaderObject::get_cuda_interop_buffers(std::vector<ref<cuda::InteropBuffer>>& cuda_interop_buffers) const
{
    ShaderObject::get_cuda_interop_buffers(cuda_interop_buffers);
    for (auto& [_, sub_object] : m_sub_objects) {
        sub_object->get_cuda_interop_buffers(cuda_interop_buffers);
    }
}

} // namespace sgl
