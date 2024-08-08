// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/resource.h"

#include <memory>
#include <vector>
#include <deque>

namespace sgl {

struct MemoryHeapDesc {
    /// The memory type of the heap.
    MemoryType memory_type{MemoryType::upload};
    /// The resource usage of the heap.
    ResourceUsage usage{ResourceUsage::none};
    /// The size of a page in bytes.
    DeviceSize page_size{4 * 1024 * 1024};
    /// True to retain large pages, false to release them after use.
    bool retain_large_pages{false};
    /// The debug name of the heap.
    std::string debug_name;
};

/**
 * \brief A memory heap is used to allocate temporary host-visible memory.
 *
 * A memory heap is a collection of memory pages. Each page has a buffer of size \c page_size.
 * When allocating memory, the heap tries to add the allocation to the current page.
 * If the allocation does not fit, a new page is allocated.
 * For allocations larger than the configured page size, a new large page is allocated.
 *
 * The memory heap is tied to a fence. Each allocation records the currently signaled fence
 * value when it is created. On release, the allocation is put on a deferred release queue.
 * Only if the fence value of the memory heap is greater than the fence value of the allocation,
 * the allocation is actually freed. This ensures that memory is not freed while still in use
 * by the device.
 *
 * Allocations are returned as unique pointers. When the pointer is destroyed, the allocation
 * is released. This ensures that the memory is freed when it is no longer used.
 */
class SGL_API MemoryHeap : public DeviceResource {
    SGL_OBJECT(MemoryHeap)
public:
    SGL_NON_COPYABLE_AND_MOVABLE(MemoryHeap);

    using PageID = uint64_t;

    struct AllocationData {
        /// The heap this allocation belongs to.
        const ref<MemoryHeap> heap;
        /// The buffer this allocation belongs to.
        const ref<Buffer> buffer;
        /// The size of the allocation.
        const DeviceSize size;
        /// The offset of the allocation within the buffer.
        const DeviceOffset offset;
        /// Pointer to the host-visible memory.
        uint8_t* const data;

        /// The page where the allocation is stored.
        PageID page_id;
        /// The signaled fence value at the time when the allocation was created.
        const uint64_t fence_value;

        ~AllocationData() { heap->release(this); }

        /// The device address of the allocation.
        DeviceAddress device_address() const { return buffer->device_address() + offset; }
    };

    using Allocation = std::unique_ptr<AllocationData>;

    struct Stats {
        /// The total size of the heap.
        DeviceSize total_size{0};
        /// The used size of the heap.
        DeviceSize used_size{0};
        /// The number of pages in the heap.
        uint32_t page_count{0};
        /// The number of large pages in the heap.
        uint32_t large_page_count{0};
    };

    MemoryHeap(ref<Device> device, ref<Fence> fence, MemoryHeapDesc desc);
    ~MemoryHeap();

    /// Description of the heap.
    const MemoryHeapDesc& desc() const { return m_desc; }
    /// Statistics of the heap.
    const Stats& stats() const { return m_stats; }

    /**
     * \brief Allocate memory from this heap.
     *
     * \param size The number of bytes to allocate.
     * \param alignment The alignment of the allocation.
     * \return Returns a unique pointer to the allocation.
     */
    Allocation allocate(DeviceSize size, DeviceSize alignment = 16);

    /**
     * \brief Execute deferred releases.
     *
     * This function should be called regularly to execute deferred releases.
     */
    void execute_deferred_releases();

    std::string to_string() const override;

private:
    void release(AllocationData* allocation);

    PageID allocate_page(DeviceSize size);
    void free_page(PageID page_id);

    PageID reclaim_or_allocate_page();

    struct Page {
        ref<Buffer> buffer;
        uint8_t* data{nullptr};
        uint32_t allocation_count{0};
        DeviceOffset current_offset{0};

        void reset()
        {
            allocation_count = 0;
            current_offset = 0;
        }
    };

    struct DeferredRelease {
        uint64_t fence_value;
        PageID page_id;
        DeviceSize size;
    };

    static constexpr PageID INVALID_PAGE = PageID(-1);

    ref<Fence> m_fence;
    MemoryHeapDesc m_desc;
    Stats m_stats;

    std::vector<Page> m_pages;
    std::vector<PageID> m_free_pages;
    std::vector<PageID> m_available_pages;
    PageID m_current_page{INVALID_PAGE};

    std::deque<DeferredRelease> m_deferred_releases;
};

} // namespace sgl
