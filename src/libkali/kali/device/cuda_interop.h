// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/core/object.h"

#include "kali/device/fwd.h"

namespace kali::cuda {

struct TensorView {
    void* data;
    size_t size;
    size_t stride;
};

class InteropBuffer : public Object {
public:
    InteropBuffer(kali::Device* device, const TensorView tensor_view, bool is_uav);
    ~InteropBuffer();

    Buffer* buffer() const { return m_buffer; }
    bool is_uav() const { return m_is_uav; }
    ref<ResourceView> get_resource_view() const;

    void copy_from_cuda(void* cuda_stream = 0);
    void copy_to_cuda(void* cuda_stream = 0);

private:
    kali::Device* m_device;
    TensorView m_tensor_view;
    bool m_is_uav;
    ref<kali::Buffer> m_buffer;
    ref<cuda::ExternalMemory> m_external_memory;
    ref<kali::ResourceView> m_srv;
    ref<kali::ResourceView> m_uav;
};

} // namespace kali::cuda
