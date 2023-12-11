#include "nanobind.h"

#include "kali/device/pipeline.h"

KALI_PY_EXPORT(device_pipeline)
{
    using namespace kali;

    nb::class_<Pipeline, DeviceResource>(m, "Pipeline");

    nb::class_<ComputePipeline, Pipeline>(m, "ComputePipeline")
        .def_prop_ro("thread_group_size", &ComputePipeline::thread_group_size);
}
