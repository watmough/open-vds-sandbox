import os
from pathlib import Path
from typing import Tuple

import pytest
import openvds

from segyimport_test_config import test_data_dir, ImportExecutor, TempVDSGuard, TempScanFileGuard


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_mutes_test")


@pytest.fixture
def azimuth_degrees_segy() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "WAZ", "waz_trial.segy")


@pytest.fixture
def azimuth_offset_xy_segy() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "Teleport", "Teleport_Trim",
                        "waz", "CG3_090008B32871_subset.segy")


@pytest.fixture
def azimuth_from_azimuth_executor(azimuth_degrees_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Fixture to setup an ImportExecutor with options for SEGY with azimuth trace header field"""
    ex = ImportExecutor()

    ex.add_args(["--header-field", "azimuth=25:4"])
    ex.add_arg("--azimuth")
    ex.add_args(["--azimuth-type", "angle"])
    ex.add_args(["--azimuth-unit", "degrees"])
    ex.add_arg("--prestack")
    # TODO disable trace order by offset?
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(azimuth_degrees_segy)

    return ex, output_vds


@pytest.fixture
def azimuth_from_offset_xy_executor(azimuth_offset_xy_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Fixture to setup an ImportExecutor with common options for SEGY with mutes"""
    ex = ImportExecutor()

    ex.add_args(["--header-field", "offsetx=97:2"])
    ex.add_args(["--header-field", "offsety=95:2"])
    ex.add_arg("--azimuth")
    ex.add_args(["--azimuth-type", "offsetxy"])
    ex.add_arg("--prestack")
    # TODO disable trace order by offset?
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(azimuth_offset_xy_segy)

    return ex, output_vds


def test_azimuth_channel_descriptor(azimuth_from_azimuth_executor):
    ex, output_vds = azimuth_from_azimuth_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        mutes_channel = layout.getChannelIndex("Azimuth")
        assert mutes_channel > 0


def test_azimuth_range_from_azimuth(azimuth_from_azimuth_executor):
    ex, output_vds = azimuth_from_azimuth_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        access_manager = openvds.getAccessManager(handle)
        layout = openvds.getLayout(handle)

        # TODO check axis dimensions?

        azimuth_channel = layout.getChannelIndex("Azimuth")
        assert azimuth_channel > 0

        trace_channel = layout.getChannelIndex("Trace")
        assert trace_channel > 0

        dim2_size = layout.getDimensionNumSamples(2)
        dim1_size = layout.getDimensionNumSamples(1)

        for dim3 in range(layout.getDimensionNumSamples(3)):
            # read one inline of trace flags and azimuths

            voxel_min = (0, 0, 0, dim3, 0, 0)
            trace_voxel_max = (1, dim1_size, dim2_size, dim3 + 1, 1, 1)

            trace_flag_request = access_manager.requestVolumeSubset(voxel_min,
                                                                    trace_voxel_max,
                                                                    channel=trace_channel,
                                                                    format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                    dimensionsND=openvds.DimensionsND.Dimensions_012)
            azimuth_request = access_manager.requestVolumeSubset(voxel_min,
                                                                 trace_voxel_max,
                                                                 channel=azimuth_channel,
                                                                 format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                 mensionsND=openvds.DimensionsND.Dimensions_012)

            trace_data = trace_flag_request.data.reshape(dim2_size, dim1_size)
            azimuth_data = azimuth_request.data.reshape(dim2_size, dim1_size)

            for dim2 in range(dim2_size):
                for dim1 in range(dim1_size):
                    if trace_data[dim2, dim1] > 0:
                        azimuth_value = azimuth_data[dim2, dim1]
                        assert 1.0 < azimuth_value <= 107.0,\
                            f"azimuth value {azimuth_value}  dim1 {dim1}  dim2 {dim2}  dim3 {dim3}"


def test_azimuth_range_from_offset_xy(azimuth_from_offset_xy_executor):
    ex, output_vds = azimuth_from_offset_xy_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        access_manager = openvds.getAccessManager(handle)
        layout = openvds.getLayout(handle)

        # TODO check axis dimensions?

        azimuth_channel = layout.getChannelIndex("Azimuth")
        assert azimuth_channel > 0

        trace_channel = layout.getChannelIndex("Trace")
        assert trace_channel > 0

        dim2_size = layout.getDimensionNumSamples(2)
        dim1_size = layout.getDimensionNumSamples(1)

        for dim3 in range(layout.getDimensionNumSamples(3)):
            # read one inline of trace flags and azimuths

            voxel_min = (0, 0, 0, dim3, 0, 0)
            trace_voxel_max = (1, dim1_size, dim2_size, dim3 + 1, 1, 1)

            trace_flag_request = access_manager.requestVolumeSubset(voxel_min,
                                                                    trace_voxel_max,
                                                                    channel=trace_channel,
                                                                    format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                    dimensionsND=openvds.DimensionsND.Dimensions_012)
            azimuth_request = access_manager.requestVolumeSubset(voxel_min,
                                                                 trace_voxel_max,
                                                                 channel=azimuth_channel,
                                                                 format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                 mensionsND=openvds.DimensionsND.Dimensions_012)

            trace_data = trace_flag_request.data.reshape(dim2_size, dim1_size)
            azimuth_data = azimuth_request.data.reshape(dim2_size, dim1_size)

            for dim2 in range(dim2_size):
                for dim1 in range(dim1_size):
                    if trace_data[dim2, dim1] > 0:
                        azimuth_value = azimuth_data[dim2, dim1]
                        assert 3.691 <= azimuth_value <= 356.31, \
                            f"azimuth value {azimuth_value}  dim1 {dim1}  dim2 {dim2}  dim3 {dim3}"


def test_samples_with_azimuth(azimuth_from_azimuth_executor):
    """Tests that samples can be read when the azimuth channel is enabled"""
    ex, output_vds = azimuth_from_azimuth_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        access_manager = openvds.getAccessManager(handle)
        layout = openvds.getLayout(handle)

        assert False, "not implemented"
