// SPDX-License-Identifier: Apache-2.0

#include "memory_heap.h"

#include "kali/device/device.h"
#include "kali/device/fence.h"

#include "kali/core/error.h"
#include "kali/core/maths.h"
#include "kali/core/string.h"

namespace kali {

MemoryHeap::MemoryHeap(ref<Device> device, ref<Fence> fence, MemoryHeapDesc desc)
    : DeviceResource(std::move(device))
    , m_fence(std::move(fence))
    , m_desc(std::move(desc))
{
    KALI_CHECK(m_desc.page_size > 0, "Invalid page size, must be larger than 0");
    KALI_CHECK(
        m_desc.memory_type == MemoryType::upload || m_desc.memory_type == MemoryType::read_back,
        "Invalid memory type, must be upload or read_back"
    );
}

MemoryHeap::~MemoryHeap()
{
    execute_deferred_releases();
    if (!m_deferred_releases.empty()) {
        log_warn(
            "MemoryHeap \"{}\" has {} unreleased allocations ({} in total)",
            m_desc.debug_name,
            m_deferred_releases.size(),
            string::format_byte_size(m_stats.used_size)
        );
    }
}

MemoryHeap::Allocation MemoryHeap::allocate(DeviceSize size, DeviceSize alignment)
{
    PageID page_id = INVALID_PAGE;

    if (size > m_desc.page_size) {
        // Allocation exceeds page size -> allocate a new large page.
        page_id = allocate_page(size);
    } else {
        // Try to add allocation to current page.
        if (m_current_page != INVALID_PAGE) {
            Page& page = m_pages[m_current_page];
            DeviceOffset aligned_offset = align_to(alignment, page.current_offset);
            if (aligned_offset + size <= page.buffer->size()) {
                page_id = m_current_page;
                page.current_offset = aligned_offset;
            }
        }
        // Otherwise, reclaim or allocate a page.
        if (page_id == INVALID_PAGE) {
            page_id = reclaim_or_allocate_page();
            m_current_page = page_id;
        }
    }

    KALI_ASSERT(page_id != INVALID_PAGE);

    Page& page = m_pages[page_id];

    Allocation allocation(new AllocationData{
        .heap = ref<MemoryHeap>(this),
        .buffer = page.buffer,
        .size = size,
        .offset = page.current_offset,
        .data = page.data + page.current_offset,
        .page_id = page_id,
        .fence_value = m_fence->signaled_value(),
    });

    page.current_offset += size;
    page.allocation_count += 1;

    // Update stats.
    m_stats.used_size += size;

    return allocation;
}

void MemoryHeap::execute_deferred_releases()
{
    if (m_deferred_releases.empty())
        return;

    uint64_t current_value = m_fence->current_value();

    while (!m_deferred_releases.empty() && m_deferred_releases.front().fence_value < current_value) {
        const DeferredRelease& deferred_release = m_deferred_releases.front();
        Page& page = m_pages[deferred_release.page_id];
        page.allocation_count -= 1;
        if (page.allocation_count == 0) {
            if (page.buffer->size() > m_desc.page_size && !m_desc.retain_large_pages)
                free_page(deferred_release.page_id);
            else
                m_available_pages.push_back(deferred_release.page_id);
        }
        m_stats.used_size -= deferred_release.size;
        m_deferred_releases.pop_front();
    }
}

std::string MemoryHeap::to_string() const
{
    return fmt::format(
        "MemoryHeap(\n"
        "  device = {},\n"
        "  fence = {},\n"
        "  memory_type = {},\n"
        "  usage = {},\n"
        "  page_size = {},\n"
        "  retain_large_pages = {},\n"
        "  debug_name = {}\n"
        ")",
        m_device,
        m_fence,
        m_desc.memory_type,
        m_desc.usage,
        string::format_byte_size(m_desc.page_size),
        m_desc.retain_large_pages,
        m_desc.debug_name
    );
}

void MemoryHeap::release(AllocationData* allocation)
{
    m_deferred_releases.push_back(DeferredRelease{
        .fence_value = allocation->fence_value,
        .page_id = allocation->page_id,
        .size = allocation->size,
    });
}

MemoryHeap::PageID MemoryHeap::allocate_page(DeviceSize size)
{
    KALI_ASSERT(size > 0);

    PageID page_id = INVALID_PAGE;

    if (m_free_pages.empty()) {
        page_id = m_pages.size();
        m_pages.resize(m_pages.size() + 1);
    } else {
        page_id = m_free_pages.back();
        m_free_pages.pop_back();
    }

    ref<Buffer> buffer = m_device->create_buffer({
        .size = size,
        .usage = m_desc.usage,
        .memory_type = m_desc.memory_type,
        .debug_name = fmt::format("{}[page_id={}]", m_desc.debug_name, page_id),
    });
    buffer->break_strong_reference_to_device();
    m_pages[page_id] = {
        .buffer = buffer,
        .data = buffer->map<uint8_t>(),
    };

    KALI_ASSERT(page_id < m_pages.size());

    // Update stats.
    m_stats.total_size += size;
    if (size > m_desc.page_size)
        m_stats.large_page_count++;
    else
        m_stats.page_count++;

    return page_id;
}

void MemoryHeap::free_page(PageID page_id)
{
    KALI_ASSERT(page_id < m_pages.size());

    Page& page = m_pages[page_id];

    // Update stats.
    m_stats.total_size -= page.buffer->size();
    if (page.buffer->size() > m_desc.page_size)
        m_stats.large_page_count--;
    else
        m_stats.page_count--;

    page.buffer->unmap();
    page = {};

    m_free_pages.push_back(page_id);
}

MemoryHeap::PageID MemoryHeap::reclaim_or_allocate_page()
{
    if (m_available_pages.empty()) {
        return allocate_page(m_desc.page_size);
    } else {
        PageID page_id = m_available_pages.back();
        m_available_pages.pop_back();
        KALI_ASSERT(page_id < m_pages.size());
        m_pages[page_id].reset();
        return page_id;
    }
}

} // namespace kali
