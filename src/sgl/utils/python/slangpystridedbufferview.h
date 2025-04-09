// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vector>
#include <map>

#include "nanobind.h"

#include "sgl/core/macros.h"
#include "sgl/core/fwd.h"
#include "sgl/core/object.h"

#include "sgl/device/fwd.h"
#include "sgl/device/resource.h"

#include "sgl/utils/python/slangpy.h"

namespace sgl::slangpy {

struct StridedBufferViewDesc {
    ref<NativeSlangType> dtype;
    ref<TypeLayoutReflection> element_layout;
    int offset{0};
    Shape shape;
    Shape strides;
    ResourceUsage usage{ResourceUsage::shader_resource | ResourceUsage::unordered_access};
    MemoryType memory_type{MemoryType::device_local};
};

class StridedBufferView : public NativeObject {
public:
    StridedBufferView(Device* device, const StridedBufferViewDesc& desc, ref<Buffer> storage);
    virtual ~StridedBufferView() {}

    virtual StridedBufferViewDesc &desc() { SGL_THROW("desc() is not implemented"); }
    virtual const StridedBufferViewDesc &desc() const { SGL_THROW("desc() is not implemented"); }

    Device* device() const { return storage()->device(); }
    ref<NativeSlangType> dtype() const { return m_desc.dtype; }
    int offset() const { return m_desc.offset; }
    const Shape& shape() const { return m_desc.shape; }
    const Shape& strides() const { return m_desc.strides; }
    int dims() const { return (int)m_desc.shape.size(); }
    size_t element_count() const { return m_desc.shape.element_count(); }
    ResourceUsage usage() const { return m_desc.usage; }
    MemoryType memory_type() const { return m_desc.memory_type; }
    ref<Buffer> storage() const { return m_storage; }
    size_t element_stride() const { return m_desc.element_layout->stride(); }

    ref<BufferCursor> cursor(std::optional<int> start = std::nullopt, std::optional<int> count = std::nullopt) const;
    nb::dict uniforms() const;

    /// Clear buffer to 0s
    void clear(CommandBuffer* cmd = nullptr);

    /// Copy to CPU memory as a numpy array of correct stride/shape
    nb::ndarray<nb::numpy> to_numpy() const;
    /// Copy from CPU memory (as a numpy array) into GPU buffer
    void copy_from_numpy(nb::ndarray<nb::numpy> data);

protected:
    /// In-place versions of view changing methods, to be called in derived classes.
    void view_inplace(Shape shape, Shape strides = Shape(), int offset = 0);
    void broadcast_to_inplace(const Shape& shape);
    void index_inplace(nb::args args);

private:
    StridedBufferViewDesc m_desc;
    ref<Buffer> m_storage;
};

} // namespace sgl::slangpy
