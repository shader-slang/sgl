// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/object.h"

#include "sgl/device/fwd.h"

namespace sgl::cuda {

struct TensorView {
    void* data;
    size_t size;
    size_t stride;
};

class InteropBuffer : public Object {
public:
    InteropBuffer(sgl::Device* device, const TensorView tensor_view, bool is_uav);
    ~InteropBuffer();

    Buffer* buffer() const { return m_buffer; }
    bool is_uav() const { return m_is_uav; }
    ref<ResourceView> get_resource_view() const;

    void copy_from_cuda(void* cuda_stream = 0);
    void copy_to_cuda(void* cuda_stream = 0);

private:
    sgl::Device* m_device;
    TensorView m_tensor_view;
    bool m_is_uav;
    ref<sgl::Buffer> m_buffer;
    ref<cuda::ExternalMemory> m_external_memory;
    ref<sgl::ResourceView> m_srv;
    ref<sgl::ResourceView> m_uav;
};

} // namespace sgl::cuda
