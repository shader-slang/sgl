#include "shader_object.h"

#include "kali/device/reflection.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"
#include "kali/device/helpers.h"
#include "kali/device/command.h"
#include "kali/device/shader.h"
#include "kali/device/device.h"
#include "kali/device/cuda_interop.h"

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

ShaderObject::ShaderObject(ref<Device> device, gfx::IShaderObject* shader_object)
    : m_device(std::move(device))
    , m_shader_object(shader_object)
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

void ShaderObject::set_acceleration_structure(
    const ShaderOffset& offset,
    const ref<AccelerationStructure>& acceleration_structure
)
{
    SLANG_CALL(
        m_shader_object->setResource(gfx_shader_offset(offset), acceleration_structure->gfx_acceleration_structure())
    );
}

void ShaderObject::set_data(const ShaderOffset& offset, void const* data, size_t size)
{
    SLANG_CALL(m_shader_object->setData(gfx_shader_offset(offset), data, size));
}

#if KALI_HAS_CUDA
void ShaderObject::set_cuda_tensor_view(const ShaderOffset& offset, const cuda::TensorView& tensor_view, bool is_uav)
{
    KALI_CHECK(m_device->supports_cuda_interop(), "Device does not support CUDA interop");
    ref<cuda::InteropBuffer> cuda_interop_buffer = make_ref<cuda::InteropBuffer>(m_device, tensor_view, is_uav);
    set_resource(offset, cuda_interop_buffer->get_resource_view());
    m_cuda_interop_buffers.push_back(cuda_interop_buffer);
}

void ShaderObject::get_cuda_interop_buffers(std::vector<ref<cuda::InteropBuffer>>& cuda_interop_buffers) const
{
    cuda_interop_buffers
        .insert(cuda_interop_buffers.end(), m_cuda_interop_buffers.begin(), m_cuda_interop_buffers.end());
}
#endif // KALI_HAS_CUDA

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
    if (m_command_buffer) {
        if (ref<MutableShaderObject> mutable_object = dynamic_ref_cast<MutableShaderObject>(object)) {
            mutable_object->set_resource_states(m_command_buffer);
        }
    }

    ShaderObject::set_object(offset, object);
}

void TransientShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    if (m_command_buffer) {
        switch (resource_view->type()) {
        case ResourceViewType::unknown:
            break;
        case ResourceViewType::render_target:
            m_command_buffer->resource_barrier(resource_view->resource(), ResourceState::render_target);
            break;
        case ResourceViewType::depth_stencil:
            m_command_buffer->resource_barrier(resource_view->resource(), ResourceState::render_target);
            break;
        case ResourceViewType::shader_resource:
            m_command_buffer->resource_barrier(resource_view->resource(), ResourceState::shader_resource);
            break;
        case ResourceViewType::unordered_access:
            m_command_buffer->resource_barrier(resource_view->resource(), ResourceState::unordered_access);
            break;
        }
    }

    ShaderObject::set_resource(offset, resource_view);
}

#if KALI_HAS_CUDA
void TransientShaderObject::get_cuda_interop_buffers(std::vector<ref<cuda::InteropBuffer>>& cuda_interop_buffers) const
{
    ShaderObject::get_cuda_interop_buffers(cuda_interop_buffers);
    for (const auto& sub_object : m_sub_objects)
        sub_object->get_cuda_interop_buffers(cuda_interop_buffers);
}
#endif // KALI_HAS_CUDA

//
// MutableShaderObject
//

MutableShaderObject::MutableShaderObject(ref<Device> device, gfx::IShaderObject* shader_object)
    : ShaderObject(std::move(device), shader_object)
{
}

MutableShaderObject::MutableShaderObject(ref<Device> device, const ShaderProgram* shader_program)
    : ShaderObject(std::move(device), nullptr)
{
    m_device->gfx_device()->createMutableRootShaderObject(shader_program->gfx_shader_program(), &m_shader_object);
}

MutableShaderObject::MutableShaderObject(ref<Device> device, const TypeLayoutReflection* type_layout)
    : ShaderObject(std::move(device), nullptr)
{
    m_device->gfx_device()->createMutableShaderObjectFromTypeLayout(type_layout->base(), &m_shader_object);
}

MutableShaderObject::~MutableShaderObject()
{
    m_shader_object->release();
}

ref<ShaderObject> MutableShaderObject::get_entry_point(uint32_t index)
{
    KALI_UNUSED(index);
    return nullptr;
}

ref<ShaderObject> MutableShaderObject::get_object(const ShaderOffset& offset)
{
    auto object = make_ref<MutableShaderObject>(m_device, m_shader_object->getObject(gfx_shader_offset(offset)));
    m_sub_objects.push_back(object);
    return object;
}

void MutableShaderObject::set_resource(const ShaderOffset& offset, const ref<ResourceView>& resource_view)
{
    ShaderObject::set_resource(offset, resource_view);

    if (resource_view)
        m_resource_views[offset] = resource_view;
    else
        m_resource_views.erase(offset);
}

void MutableShaderObject::set_resource_states(CommandBuffer* command_buffer)
{
    for (auto& [offset, resource_view] : m_resource_views) {
        switch (resource_view->type()) {
        case ResourceViewType::unknown:
            break;
        case ResourceViewType::render_target:
            command_buffer->resource_barrier(resource_view->resource(), ResourceState::render_target);
            break;
        case ResourceViewType::depth_stencil:
            command_buffer->resource_barrier(resource_view->resource(), ResourceState::render_target);
            break;
        case ResourceViewType::shader_resource:
            command_buffer->resource_barrier(resource_view->resource(), ResourceState::shader_resource);
            break;
        case ResourceViewType::unordered_access:
            command_buffer->resource_barrier(resource_view->resource(), ResourceState::unordered_access);
            break;
        }
    }
}

#if KALI_HAS_CUDA
void MutableShaderObject::get_cuda_interop_buffers(std::vector<ref<cuda::InteropBuffer>>& cuda_interop_buffers) const
{
    ShaderObject::get_cuda_interop_buffers(cuda_interop_buffers);
    for (const auto& sub_object : m_sub_objects)
        sub_object->get_cuda_interop_buffers(cuda_interop_buffers);
}
#endif // KALI_HAS_CUDA

} // namespace kali
