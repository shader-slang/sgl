import pytest
import sgl
import sys
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers

COOPVEC_DEVICE_TYPES = [sgl.DeviceType.vulkan]


def get_coop_vec_device(device_type: sgl.DeviceType) -> sgl.Device:
    if sys.platform == "darwin":
        pytest.skip("Cooperative vector tests not supported on MacOS")

    device = helpers.get_device(device_type)
    if not "cooperative-vector" in device.features:
        pytest.skip("Device does not support cooperative vector")
    return device


SIZE_CHECKS = [
    (4, 4, sgl.DataType.float32, 64),
    (4, 8, sgl.DataType.float32, 128),
    (8, 4, sgl.DataType.float32, 128),
    (4, 4, sgl.DataType.float16, 32),
    (4, 8, sgl.DataType.float16, 64),
    (8, 4, sgl.DataType.float16, 64),
]


@pytest.mark.parametrize("device_type", COOPVEC_DEVICE_TYPES)
@pytest.mark.parametrize("rows, cols, dtype, expected_size", SIZE_CHECKS)
def test_matrix_size(
    device_type: sgl.DeviceType,
    rows: int,
    cols: int,
    dtype: sgl.DataType,
    expected_size: int,
):
    device = get_coop_vec_device(device_type)

    sz = device.coopvec_query_matrix_size(
        rows, cols, sgl.CoopVecMatrixLayout.row_major, dtype
    )
    assert sz == expected_size

    sz = device.coopvec_query_matrix_size(
        rows, cols, sgl.CoopVecMatrixLayout.column_major, dtype
    )
    assert sz == expected_size


@pytest.mark.parametrize("device_type", COOPVEC_DEVICE_TYPES)
@pytest.mark.parametrize("rows", [4, 8])
@pytest.mark.parametrize("cols", [4, 8])
@pytest.mark.parametrize("dtype", [sgl.DataType.float32, sgl.DataType.float16])
@pytest.mark.parametrize(
    "layout",
    [
        sgl.CoopVecMatrixLayout.row_major,
        sgl.CoopVecMatrixLayout.column_major,
        sgl.CoopVecMatrixLayout.inferencing_optimal,
        sgl.CoopVecMatrixLayout.training_optimal,
    ],
)
def test_matrix_desc(
    device_type: sgl.DeviceType,
    rows: int,
    cols: int,
    dtype: sgl.DataType,
    layout: sgl.CoopVecMatrixLayout,
):
    device = get_coop_vec_device(device_type)
    desc = device.coopvec_create_matrix_desc(rows, cols, layout, dtype)
    size = device.coopvec_query_matrix_size(rows, cols, layout, dtype)
    assert desc.rows == rows
    assert desc.cols == cols
    assert desc.layout == layout
    assert desc.size == size
    assert desc.element_type == dtype


@pytest.mark.parametrize("device_type", COOPVEC_DEVICE_TYPES)
def test_matrix_convert_matrix_host(device_type: sgl.DeviceType):
    device = get_coop_vec_device(device_type)

    data = np.array([[1, 2, 3, 4], [5, 6, 7, 8]], dtype=np.float32)
    result = np.zeros_like(data)

    device.coopvec_convert_matrix_host(
        data,
        result,
        sgl.CoopVecMatrixLayout.row_major,
        sgl.CoopVecMatrixLayout.column_major,
    )

    # Conversion should have changed memory layout, but not shape
    data_t = np.transpose(data)
    result = result.reshape(data_t.shape)
    assert np.allclose(data_t, result)


@pytest.mark.parametrize("device_type", COOPVEC_DEVICE_TYPES)
def test_matrix_convert_huge_matrix_host(device_type: sgl.DeviceType):
    device = get_coop_vec_device(device_type)

    data = np.random.random((128, 128)).astype(np.float32)
    result = np.zeros_like(data)

    device.coopvec_convert_matrix_host(
        data,
        result,
        sgl.CoopVecMatrixLayout.row_major,
        sgl.CoopVecMatrixLayout.column_major,
    )

    # Conversion should have changed memory layout, but not shape
    data_t = np.transpose(data)
    result = result.reshape(data_t.shape)
    assert np.allclose(data_t, result)


@pytest.mark.parametrize("device_type", COOPVEC_DEVICE_TYPES)
def test_matrix_convert_matrix_device(device_type: sgl.DeviceType):
    device = get_coop_vec_device(device_type)

    data = np.random.random((128, 128)).astype(np.float32)

    data_buf = device.create_buffer(data=data, usage=sgl.BufferUsage.shader_resource)
    result_buf = device.create_buffer(
        element_count=data.size,
        struct_size=4,
        usage=sgl.BufferUsage.shader_resource | sgl.BufferUsage.unordered_access,
    )

    data_desc = device.coopvec_create_matrix_desc(
        data.shape[0],
        data.shape[1],
        sgl.CoopVecMatrixLayout.row_major,
        sgl.DataType.float32,
    )
    result_desc = device.coopvec_create_matrix_desc(
        data.shape[0],
        data.shape[1],
        sgl.CoopVecMatrixLayout.column_major,
        sgl.DataType.float32,
    )

    device.coopvec_convert_matrix_device(data_buf, data_desc, result_buf, result_desc)

    result = result_buf.to_numpy().view(np.float32).reshape(data.shape)

    data_t = np.transpose(data)
    result = result.reshape(
        data_t.shape
    )  # probably shouldn't have to do this but conversion doesn't change shape
    assert np.allclose(data_t, result)


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
