// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/memory_heap.h"

SGL_PY_EXPORT(device_memory_heap)
{
    using namespace sgl;

    nb::class_<MemoryHeapDesc>(m, "MemoryHeapDesc", D(MemoryHeapDesc))
        .def(nb::init<>())
        .def_rw("memory_type", &MemoryHeapDesc::memory_type, D(MemoryHeapDesc, memory_type))
        .def_rw("usage", &MemoryHeapDesc::usage, D(MemoryHeapDesc, usage))
        .def_rw("page_size", &MemoryHeapDesc::page_size, D(MemoryHeapDesc, page_size))
        .def_rw("retain_large_pages", &MemoryHeapDesc::retain_large_pages, D(MemoryHeapDesc, retain_large_pages))
        .def_rw("debug_name", &MemoryHeapDesc::debug_name, D(MemoryHeapDesc, debug_name));

    nb::class_<MemoryHeap, DeviceResource> memory_heap(m, "MemoryHeap", D(MemoryHeap));

    nb::class_<MemoryHeap::AllocationData>(memory_heap, "Allocation", D(MemoryHeap, AllocationData))
        .def_ro("buffer", &MemoryHeap::AllocationData::buffer, D(MemoryHeap, AllocationData, buffer))
        .def_ro("size", &MemoryHeap::AllocationData::size, D(MemoryHeap, AllocationData, size))
        .def_ro("offset", &MemoryHeap::AllocationData::offset, D(MemoryHeap, AllocationData, offset))
        .def_prop_ro(
            "device_address",
            &MemoryHeap::AllocationData::device_address,
            D(MemoryHeap, AllocationData, device_address)
        );

    nb::class_<MemoryHeap::Stats>(memory_heap, "Stats", D(MemoryHeap, Stats))
        .def_ro("total_size", &MemoryHeap::Stats::total_size, D(MemoryHeap, Stats, total_size))
        .def_ro("used_size", &MemoryHeap::Stats::used_size, D(MemoryHeap, Stats, used_size))
        .def_ro("page_count", &MemoryHeap::Stats::page_count, D(MemoryHeap, Stats, page_count))
        .def_ro("large_page_count", &MemoryHeap::Stats::large_page_count, D(MemoryHeap, Stats, large_page_count));

    memory_heap //
        .def("allocate", &MemoryHeap::allocate, "size"_a, "alignment"_a = 1, D(MemoryHeap, allocate))
        .def_prop_ro("stats", &MemoryHeap::stats, D(MemoryHeap, stats));
}
