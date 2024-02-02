from typing import List, Tuple

import pytest
import os
import openvds
from pathlib import Path
import numpy as np

from segyimport_test_config import ImportExecutor, TempVDSGuard, segyimport_test_data_dir


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import2d_trace_positions_test")


@pytest.fixture
def output_vds_two() -> TempVDSGuard:
    return TempVDSGuard("import2d_trace_positions_test_two")


@pytest.fixture
def normal_segy(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "filt_mig.sgy")


@pytest.fixture
def falling_inline_segy(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "TeapotDomeAlternateSorting", "filt_mig_falling_inline.sgy")


@pytest.fixture
def falling_crossline_segy(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "TeapotDomeAlternateSorting", "filt_mig_falling_crossline.sgy")


@pytest.fixture
def falling_inline_falling_crossline_segy(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "TeapotDomeAlternateSorting", "filt_mig_falling_inline_crossline.sgy")


@pytest.fixture
def axis_info() -> List[Tuple[str, str, float, float, int]]:
    return [
        ("Sample", "ms", 0.0, 3000.0, 1501),
        ("Crossline", "unitless", 1.0, 188.0, 188),
        ("Inline", "unitless", 1.0, 345.0, 345)
    ]


def set_filt_mig_parameters(ex: ImportExecutor, keep_original_order: bool = False):
    ex.add_args(["--header-field", "inlinenumber=181:4"])
    ex.add_args(["--header-field", "crosslinenumber=185:4"])
    ex.add_args(["--header-field", "ensemblexcoordinate=189:4"])
    ex.add_args(["--header-field", "ensembleycoordinate=193:4"])

    if keep_original_order:
        ex.add_arg("--keep-original-order")


def check_axis_info(filename: str, axis_info, inline_is_ascending: bool, crossline_is_ascending: bool):
    reverse_coords = []
    for i in range(len(axis_info)):
        reverse_coords.append(False)
    if not inline_is_ascending:
        reverse_coords[-1] = True
    if not crossline_is_ascending:
        reverse_coords[-2] = True

    with openvds.open(filename, "") as handle:
        layout = openvds.getLayout(handle)

        assert len(axis_info) == layout.dimensionality
        for axis_num in range(len(axis_info)):
            info = axis_info[axis_num]
            descriptor = layout.getAxisDescriptor(axis_num)
            assert info[0] == descriptor.name
            assert info[1] == descriptor.unit
            assert info[4] == descriptor.numSamples

            if reverse_coords[axis_num]:
                # coordinates on this axis should be decreasing, we need to swap the min/max comparisons
                assert info[2] == descriptor.coordinateMax
                assert info[3] == descriptor.coordinateMin
            else:
                assert info[2] == descriptor.coordinateMin
                assert info[3] == descriptor.coordinateMax


def compare_vds_sample_data(original_filename: str, comparison_filename: str):
    with openvds.open(original_filename, "") as original_handle, \
            openvds.open(comparison_filename, "") as comparison_handle:
        original_layout = openvds.getLayout(original_handle)
        comparison_layout = openvds.getLayout(comparison_handle)

        assert original_layout.dimensionality == comparison_layout.dimensionality

        voxel_max = [1, 1, 1, 1, 1, 1]
        data_shape = []

        for dim in range(original_layout.dimensionality):
            voxel_max[dim] = original_layout.getDimensionNumSamples(dim)
            data_shape.insert(0, original_layout.getDimensionNumSamples(dim))

            assert original_layout.getDimensionNumSamples(dim) == comparison_layout.getDimensionNumSamples(dim)

        original_access_mgr = openvds.getAccessManager(original_handle)
        comparison_access_mgr = openvds.getAccessManager(comparison_handle)

        original_request = original_access_mgr.requestVolumeSubset((0, 0, 0, 0, 0, 0), voxel_max,
                                                                   channel=0,
                                                                   format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                   dimensionsND=openvds.DimensionsND.Dimensions_012)
        comparison_request = comparison_access_mgr.requestVolumeSubset((0, 0, 0, 0, 0, 0), voxel_max,
                                                                       channel=0,
                                                                       format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                       dimensionsND=openvds.DimensionsND.Dimensions_012)

        original_data = original_request.data.reshape(data_shape)
        comparison_data = comparison_request.data.reshape(data_shape)

        np.testing.assert_array_equal(original_data, comparison_data)


def test_volume_info_normal(normal_segy, output_vds, axis_info):
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(normal_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_axis_info(output_vds.filename, axis_info, True, True)


def test_volume_info_falling_inline(falling_inline_segy, output_vds, axis_info):
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(falling_inline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_axis_info(output_vds.filename, axis_info, True, True)


def test_volume_info_falling_inline_keep_original(falling_inline_segy, output_vds, axis_info):
    ex = ImportExecutor()

    set_filt_mig_parameters(ex, keep_original_order=True)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(falling_inline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_axis_info(output_vds.filename, axis_info, False, True)


def test_volume_info_falling_crossline(falling_crossline_segy, output_vds, axis_info):
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(falling_crossline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_axis_info(output_vds.filename, axis_info, True, True)


def test_volume_info_falling_crossline_keep_original(falling_crossline_segy, output_vds, axis_info):
    ex = ImportExecutor()

    set_filt_mig_parameters(ex, keep_original_order=True)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(falling_crossline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_axis_info(output_vds.filename, axis_info, True, False)


def test_volume_info_falling_inline_falling_crossline(falling_inline_falling_crossline_segy, output_vds, axis_info):
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(falling_inline_falling_crossline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_axis_info(output_vds.filename, axis_info, True, True)


def test_volume_info_falling_inline_falling_crossline_keep_original(falling_inline_falling_crossline_segy, output_vds,
                                                                    axis_info):
    ex = ImportExecutor()

    set_filt_mig_parameters(ex, keep_original_order=True)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(falling_inline_falling_crossline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_axis_info(output_vds.filename, axis_info, False, False)


def test_compare_falling_inline(normal_segy, falling_inline_segy, output_vds, output_vds_two):
    # Convert the original SEGY
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(normal_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # Convert the falling-inline version
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds_two.filename])

    ex.add_arg(falling_inline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    compare_vds_sample_data(output_vds.filename, output_vds_two.filename)


def test_compare_falling_crossline(normal_segy, falling_crossline_segy, output_vds, output_vds_two):
    # Convert the original SEGY
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(normal_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # Convert the falling-crossline version
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds_two.filename])

    ex.add_arg(falling_crossline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    compare_vds_sample_data(output_vds.filename, output_vds_two.filename)


def test_compare_falling_inline_falling_crossline(normal_segy, falling_inline_falling_crossline_segy, output_vds,
                                                  output_vds_two):
    # Convert the original SEGY
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(normal_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # Convert the falling-inline falling-crossline version
    ex = ImportExecutor()

    set_filt_mig_parameters(ex)

    ex.add_args(["--vdsfile", output_vds_two.filename])

    ex.add_arg(falling_inline_falling_crossline_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    compare_vds_sample_data(output_vds.filename, output_vds_two.filename)
