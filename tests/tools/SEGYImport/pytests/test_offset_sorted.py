import os
from pathlib import Path
from typing import Tuple

import pytest
import openvds

from segyimport_test_config import test_data_dir, ImportExecutor, TempVDSGuard, TempScanFileGuard


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_offset_sorted_test")


@pytest.fixture
def output_scan() -> TempScanFileGuard:
    return TempScanFileGuard("ST0202.segy.test.scan.json")


@pytest.fixture
def offset_sorted_segy() -> str:
    return os.path.join(test_data_dir, "Plugins", "ImportPlugins", "SEGYUnittest", "OffsetSorted", "ST0202.segy")


@pytest.fixture
def offset_sorted_deduped_segy() -> str:
    return os.path.join(test_data_dir, "Plugins", "ImportPlugins", "SEGYUnittest", "OffsetSorted",
                        "ST0202_deduped.segy")


@pytest.fixture
def offset_sorted_scan() -> str:
    return os.path.join(test_data_dir, "Plugins", "ImportPlugins", "SEGYUnittest", "OffsetSorted",
                        "ST0202.segy.scan.json")


@pytest.fixture
def conventional_sorted_segy() -> str:
    return os.path.join(test_data_dir, "Plugins", "ImportPlugins", "SEGYUnittest", "OffsetSorted",
                        "ST0202_conventional.segy")


@pytest.fixture
def conventional_output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_conventional_sorted_test")


@pytest.fixture
def offset_sorted_executor(offset_sorted_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Fixture to setup an ImportExecutor with common options for offset-sorted SEGY"""
    ex = ImportExecutor()

    ex.add_args(["--header-field", "offset=177:4"])

    ex.add_arg("--offset-sorted")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(offset_sorted_segy)

    return ex, output_vds


@pytest.fixture
def offset_sorted_deduped_executor(offset_sorted_deduped_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Fixture to setup an ImportExecutor with common options for de-duped version of offset-sorted SEGY"""
    ex = ImportExecutor()

    ex.add_args(["--header-field", "offset=177:4"])

    ex.add_arg("--offset-sorted")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(offset_sorted_deduped_segy)

    return ex, output_vds


@pytest.fixture
def conventional_sorted_executor(conventional_sorted_segy, conventional_output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Fixture to setup an ImportExecutor with common options for conventionally sorted SEGY"""
    ex = ImportExecutor()

    ex.add_args(["--header-field", "offset=177:4"])

    ex.add_arg("--prestack")
    ex.add_args(["--vdsfile", conventional_output_vds.filename])

    ex.add_arg(conventional_sorted_segy)

    return ex, conventional_output_vds


def test_produce_status(offset_sorted_executor):
    ex, output_vds = offset_sorted_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        access_manager = openvds.getAccessManager(handle)

        assert access_manager.getVDSProduceStatus(openvds.DimensionsND.Dimensions_023) != openvds.VDSProduceStatus.Unavailable

        assert access_manager.getVDSProduceStatus(openvds.DimensionsND.Dimensions_02) == openvds.VDSProduceStatus.Unavailable

        assert access_manager.getVDSProduceStatus(openvds.DimensionsND.Dimensions_01) == openvds.VDSProduceStatus.Unavailable
        assert access_manager.getVDSProduceStatus(openvds.DimensionsND.Dimensions_03) == openvds.VDSProduceStatus.Unavailable
        assert access_manager.getVDSProduceStatus(openvds.DimensionsND.Dimensions_012) == openvds.VDSProduceStatus.Unavailable
        assert access_manager.getVDSProduceStatus(openvds.DimensionsND.Dimensions_013) == openvds.VDSProduceStatus.Unavailable


def test_survey_coordinate_system(offset_sorted_executor):
    ex, output_vds = offset_sorted_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        # verify that coordinate system metadata exists
        assert layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().category,
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().name)
        assert layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().name)
        assert layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().name)

        # verify coordinate system component values
        value = layout.getMetadataDoubleVector2(
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().category,
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().name)
        assert value[0] == pytest.approx(431961.93, abs=0.05)
        assert value[1] == pytest.approx(6348554.9, abs=0.05)

        value = layout.getMetadataDoubleVector2(
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().name)
        assert value[0] == pytest.approx(6.05, abs=0.05)
        assert value[1] == pytest.approx(24.26, abs=0.05)

        value = layout.getMetadataDoubleVector2(
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().name)
        assert value[0] == pytest.approx(-12.13, abs=0.05)
        assert value[1] == pytest.approx(3.02, abs=0.05)


def test_axes(offset_sorted_executor):
    ex, output_vds = offset_sorted_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        assert 4 == layout.getDimensionality()

        assert 1876 == layout.getDimensionNumSamples(0)
        assert 0.0 == layout.getDimensionMin(0)
        assert 7500.0 == layout.getDimensionMax(0)

        assert 65 == layout.getDimensionNumSamples(1)
        assert 1.0 == layout.getDimensionMin(1)
        assert 65.0 == layout.getDimensionMax(1)

        assert 160 == layout.getDimensionNumSamples(2)
        assert 1961.0 == layout.getDimensionMin(2)
        assert 2120.0 == layout.getDimensionMax(2)

        assert 40 == layout.getDimensionNumSamples(3)
        assert 4982.0 == layout.getDimensionMin(3)
        assert 5021.0 == layout.getDimensionMax(3)

        assert pytest.approx(-0.137178, abs=1.0e-05) == layout.getChannelValueRangeMin(0)
        assert pytest.approx(0.141104, abs=1.0e-05) == layout.getChannelValueRangeMax(0)


def test_read_gather(offset_sorted_executor):
    ex, output_vds = offset_sorted_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)
        access_manager = openvds.getAccessManager(handle)
        dim0_size = layout.getDimensionNumSamples(0)
        dim1_size = layout.getDimensionNumSamples(1)

        trace_channel = layout.getChannelIndex("Trace")
        assert trace_channel > 0

        inline_index = layout.getDimensionNumSamples(3) // 2
        crossline_index = layout.getDimensionNumSamples(2) // 2

        request = access_manager.requestVolumeSubset((0, 0, crossline_index, inline_index, 0, 0),
                                                     (dim0_size, dim1_size, crossline_index + 1, inline_index + 1, 1, 1),
                                                     channel=0,
                                                     format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                     dimensionsND=openvds.DimensionsND.Dimensions_023)
        trace_flag_request = access_manager.requestVolumeSubset((0, 0, crossline_index, inline_index, 0, 0),
                                                                (1, dim1_size, crossline_index + 1, inline_index + 1, 1, 1),
                                                                channel=trace_channel,
                                                                format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                dimensionsND=openvds.DimensionsND.Dimensions_023)

        data = request.data.reshape(dim1_size, dim0_size)
        trace_flag_data = trace_flag_request.data

        for dim1 in range(dim1_size):
            total = 0
            for dim0 in range(dim0_size):
                total += abs(data[dim1, dim0])

            if trace_flag_data[dim1] == 0:
                assert total == 0.0, f"dead trace at {dim1}"
            else:
                assert total > 0.0, f"trace at {dim1}"


@pytest.mark.skip(reason="This test is really inefficient and takes ~100 minutes to run")
def test_compare_with_conventional_sorted(offset_sorted_executor, conventional_sorted_executor):
    ex, output_vds = offset_sorted_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    conventional_ex, conventional_vds = conventional_sorted_executor

    result = conventional_ex.run()

    assert result == 0, conventional_ex.output()
    assert Path(conventional_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        with openvds.open(conventional_vds.filename, "") as handle_conv:
            access_manager = openvds.getAccessManager(handle)
            layout = openvds.getLayout(handle)

            access_manager_conv = openvds.getAccessManager(handle_conv)
            layout_conv = openvds.getLayout(handle_conv)

            num_z = layout.getDimensionNumSamples(0)
            fold = layout.getDimensionNumSamples(1)
            num_crossline = layout.getDimensionNumSamples(2)
            num_inline = layout.getDimensionNumSamples(3)

            assert num_z == layout_conv.getDimensionNumSamples(0)
            assert fold == layout_conv.getDimensionNumSamples(1)
            assert num_crossline == layout_conv.getDimensionNumSamples(2)
            assert num_inline == layout_conv.getDimensionNumSamples(3)

            trace_channel = layout.getChannelIndex("Trace")
            trace_channel_conv = layout_conv.getChannelIndex("Trace")
            offset_channel = layout.getChannelIndex("Offset")
            offset_channel_conv = layout_conv.getChannelIndex("Offset")

            assert trace_channel > 0
            assert trace_channel_conv > 0
            assert offset_channel > 0
            assert offset_channel_conv > 0

            for inline_index in range(num_inline):
                for crossline_index in range(num_crossline):
                    voxel_min = (0, 0, crossline_index, inline_index, 0, 0)
                    voxel_max = (num_z, fold, crossline_index + 1, inline_index + 1, 1, 1)
                    trace_voxel_max = (1, fold, crossline_index + 1, inline_index + 1, 1, 1)

                    request = access_manager.requestVolumeSubset(voxel_min, voxel_max,
                                                                 channel=0,
                                                                 format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                 dimensionsND=openvds.DimensionsND.Dimensions_023)
                    request_conv = access_manager_conv.requestVolumeSubset(voxel_min, voxel_max,
                                                                           channel=0,
                                                                           format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                           dimensionsND=openvds.DimensionsND.Dimensions_012)

                    trace_flag_request = access_manager.requestVolumeSubset(voxel_min, trace_voxel_max,
                                                                            channel=trace_channel,
                                                                            format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                            dimensionsND=openvds.DimensionsND.Dimensions_023)
                    trace_flag_request_conv = access_manager_conv.requestVolumeSubset(voxel_min, trace_voxel_max,
                                                                                      channel=trace_channel_conv,
                                                                                      format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                                      dimensionsND=openvds.DimensionsND.Dimensions_012)

                    offset_request = access_manager.requestVolumeSubset(voxel_min, trace_voxel_max,
                                                                        channel=offset_channel,
                                                                        format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                        dimensionsND=openvds.DimensionsND.Dimensions_023)
                    offset_request_conv = access_manager_conv.requestVolumeSubset(voxel_min, trace_voxel_max,
                                                                                  channel=offset_channel_conv,
                                                                                  format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                                  dimensionsND=openvds.DimensionsND.Dimensions_012)

                    sample_data = request.data.reshape(fold, num_z)
                    sample_data_conv = request_conv.data.reshape(fold, num_z)

                    for trace_index in range(fold):
                        assert trace_flag_request.data[trace_index] == trace_flag_request_conv.data[trace_index],\
                            f"trace index {trace_index}  xl index {crossline_index}  il index {inline_index}"
                        assert offset_request.data[trace_index] == offset_request_conv.data[trace_index],\
                            f"trace index {trace_index}  xl index {crossline_index}  il index {inline_index}"

                        if trace_flag_request.data[trace_index] != 0:
                            for sample_index in range(num_z):
                                assert sample_data[trace_index, sample_index] == sample_data_conv[trace_index, sample_index],\
                                    f"sample index {sample_index}  trace index {trace_index}  xl index {crossline_index}  il index {inline_index}"


def test_create_scan_file(offset_sorted_segy, output_scan):
    ex = ImportExecutor()

    ex.add_args(["--header-field", "offset=177:4"])

    ex.add_arg("--offset-sorted")
    ex.add_arg("--scan")
    ex.add_args(["--file-info", output_scan.filename])

    ex.add_arg(offset_sorted_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_scan.filename).exists()

    # might be nice to load the scan file do some light verification of contents...


def test_with_scan_file(offset_sorted_segy, offset_sorted_scan, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--header-field", "offset=177:4"])

    ex.add_arg("--offset-sorted")
    ex.add_args(["--file-info", offset_sorted_scan])
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(offset_sorted_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # check produce status as a quick test for output VDS validity
    with openvds.open(output_vds.filename, "") as handle:
        access_manager = openvds.getAccessManager(handle)

        assert access_manager.getVDSProduceStatus(openvds.DimensionsND.Dimensions_023) != openvds.VDSProduceStatus.Unavailable


def test_dupe_warning(offset_sorted_executor):
    ex, output_vds = offset_sorted_executor

    s = ex.command_line()
    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # check that the output contains the warning message for traces with duplicate offset, inline, crossline key combos
    assert "duplicate key combinations" in ex.output()


def test_dupe_no_warning(offset_sorted_deduped_executor):
    ex, output_vds = offset_sorted_deduped_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # check that the output contains the warning message for traces with duplicate offset, inline, crossline key combos
    assert "duplicate key combinations" not in ex.output()
