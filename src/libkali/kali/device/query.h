#pragma once

#include "kali/device/fwd.h"
#include "kali/device/types.h"
#include "kali/device/device_resource.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <slang-gfx.h>

#include <span>

namespace kali {

struct QueryPoolDesc {
    QueryType type;
    uint32_t count;
};

class KALI_API QueryPool : public DeviceResource {
    KALI_OBJECT(QueryPool)
public:
    QueryPool(ref<Device> device, QueryPoolDesc desc);

    const QueryPoolDesc& desc() const { return m_desc; }

    void reset();

    void get_results(uint32_t index, uint32_t count, std::span<uint64_t> result);
    std::vector<uint64_t> get_results(uint32_t index, uint32_t count);
    uint64_t get_result(uint32_t index);

    void get_timestamp_results(uint32_t index, uint32_t count, std::span<double> result);
    std::vector<double> get_timestamp_results(uint32_t index, uint32_t count);
    double get_timestamp_result(uint32_t index);

    gfx::IQueryPool* gfx_query_pool() const { return m_gfx_query_pool; }

    std::string to_string() const override;

private:
    QueryPoolDesc m_desc;
    Slang::ComPtr<gfx::IQueryPool> m_gfx_query_pool;
};

} // namespace kali