// SPDX-License-Identifier: Apache-2.0

#include "query.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"

#include "kali/core/error.h"
#include "kali/core/type_utils.h"

namespace kali {

QueryPool::QueryPool(ref<Device> device, QueryPoolDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::IQueryPool::Desc gfx_desc{
        .type = static_cast<gfx::QueryType>(m_desc.type),
        .count = narrow_cast<gfx::GfxCount>(m_desc.count),
    };
    SLANG_CALL(m_device->gfx_device()->createQueryPool(gfx_desc, m_gfx_query_pool.writeRef()));
}

void QueryPool::reset()
{
    SLANG_CALL(m_gfx_query_pool->reset());
}

void QueryPool::get_results(uint32_t index, uint32_t count, std::span<uint64_t> result)
{
    KALI_CHECK(index + count <= m_desc.count, "'index' / 'count' out of range");
    KALI_CHECK(result.size() >= count, "'result' buffer too small");
    SLANG_CALL(m_gfx_query_pool->getResult(index, count, result.data()));
}

std::vector<uint64_t> QueryPool::get_results(uint32_t index, uint32_t count)
{
    std::vector<uint64_t> result(count);
    get_results(index, count, result);
    return result;
}

uint64_t QueryPool::get_result(uint32_t index)
{
    KALI_CHECK(index < m_desc.count, "'index' out of range");
    uint64_t result;
    SLANG_CALL(m_gfx_query_pool->getResult(index, 1, &result));
    return result;
}

void QueryPool::get_timestamp_results(uint32_t index, uint32_t count, std::span<double> result)
{
    KALI_CHECK(m_desc.type == QueryType::timestamp, "QueryPool is not a timestamp query pool");
    get_results(index, count, std::span<uint64_t>(reinterpret_cast<uint64_t*>(result.data()), result.size()));
    double frequency = double(m_device->info().timestamp_frequency);
    for (uint32_t i = 0; i < count; ++i)
        result[i] = double(reinterpret_cast<uint64_t&>(result[i])) / frequency;
}

std::vector<double> QueryPool::get_timestamp_results(uint32_t index, uint32_t count)
{
    std::vector<double> result(count);
    get_timestamp_results(index, count, result);
    return result;
}

double QueryPool::get_timestamp_result(uint32_t index)
{
    KALI_CHECK(m_desc.type == QueryType::timestamp, "QueryPool is not a timestamp query pool");
    uint64_t result = get_result(index);
    return double(result) / double(m_device->info().timestamp_frequency);
}

std::string QueryPool::to_string() const
{
    return fmt::format(
        "QueryPool(\n"
        "  device = {},\n",
        "  type = {},\n",
        "  count = {}\n"
        ")",
        m_device,
        m_desc.type,
        m_desc.count
    );
}

} // namespace kali
