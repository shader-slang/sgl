#include "nanobind.h"

#include "kali/device/pipeline.h"

KALI_PY_EXPORT(device_pipeline)
{
    using namespace kali;

    nb::class_<PipelineState, DeviceResource>(m, "PipelineState");
    nb::class_<ComputePipelineState, PipelineState>(m, "ComputePipelineState");
}
