import os
from pathlib import Path
from typing import Tuple, Union

import pytest
import openvds

from segyimport_test_config import test_data_dir, ImportExecutor, TempVDSGuard


def construct_respace_executor(output_vds: TempVDSGuard, segy_name: str, respace_setting: Union[str, None]) -> ImportExecutor:
    ex = ImportExecutor()

    if respace_setting:
        ex.add_args(["--respace-gathers", respace_setting])

    ex.add_arg("--prestack")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(segy_name)

    return ex


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_mutes_test")


@pytest.fixture
def prestack_segy() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "Teleport", "Teleport_Trim",
                        "3D_Prestack", "ST0202R08_Gather_Time.segy")


@pytest.fixture
def off_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with respacing set to Off"""
    ex = construct_respace_executor(output_vds, prestack_segy, "Off")
    return ex, output_vds


@pytest.fixture
def invalid_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with respacing set to an invalid value"""
    ex = construct_respace_executor(output_vds, prestack_segy, "Partial")
    return ex, output_vds


@pytest.fixture
def prestack_vds_baseline() -> str:
    return "C:/temp/SEGY/t/ST0202R08_Gather_Time_baseline.vds"


@pytest.fixture
def auto_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with default respacing"""
    ex = construct_respace_executor(output_vds, prestack_segy, None)
    return ex, output_vds


def get_gathers_stats(vds_filename: str) -> Tuple[int, int, int, int, int, int, int]:
    """
    Read all the gathers in a single inline and return stats on how many gathers have dead traces in various
    regions of the gather.

    :return tuple of (
      total gathers,
      number of gathers with dead traces only at front of gather,
      number of gathers with dead traces only within the gather,
      number of gathers with dead traces only at end of gather,
      number of gathers with dead traces in multiple places,
      number of gathers with no dead traces
      number of consecutive duplicate offset values within a gather
      )
    """
    with openvds.open(vds_filename, "") as handle:
        layout = openvds.getLayout(handle)
        access_manager = openvds.getAccessManager(handle)

        trace_channel = layout.getChannelIndex("Trace")
        assert trace_channel > 0, "Trace channel not found"

        offset_channel = layout.getChannelIndex("Offset")
        assert offset_channel > 0, "Offset channel not found"

        # only read one inline; make it the middle inline
        inline_index = layout.getDimensionNumSamples(3) // 2
        dim2_size = layout.getDimensionNumSamples(2)
        dim1_size = layout.getDimensionNumSamples(1)

        # stats counters
        front_only = 0
        middle_only = 0
        end_only = 0
        mixed = 0
        no_dead = 0
        duplicate_offsets = 0

        for crossline_index in range(dim2_size):
            trace_flag_request = access_manager.requestVolumeSubset((0, 0, crossline_index, inline_index, 0, 0),
                                                                    (1, dim1_size, crossline_index + 1,
                                                                     inline_index + 1, 1, 1),
                                                                    channel=trace_channel,
                                                                    format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                    dimensionsND=openvds.DimensionsND.Dimensions_012)

            offset_request = access_manager.requestVolumeSubset((0, 0, crossline_index, inline_index, 0, 0),
                                                                (1, dim1_size, crossline_index + 1, inline_index + 1, 1,
                                                                 1),
                                                                channel=offset_channel,
                                                                format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                dimensionsND=openvds.DimensionsND.Dimensions_012)

            assert dim1_size == trace_flag_request.data.shape[0]
            assert dim1_size == offset_request.data.shape[0]

            # analyze trace flag and offset data to figure out which stats counters to bump

            dead_trace_ranges = []
            i = 0
            while i < dim1_size:
                if trace_flag_request.data[i] == 0:
                    start = i
                    i += 1
                    while i < dim1_size and trace_flag_request.data[i] == 0:
                        i += 1
                    end = i - 1
                    dead_trace_ranges.append((start, end))
                else:
                    i += 1

            if len(dead_trace_ranges) == 0:
                no_dead += 1
            elif len(dead_trace_ranges) == 1:
                # either front, middle, end
                start, stop = dead_trace_ranges[0]
                if start == 0:
                    front_only += 1
                elif stop == trace_flag_request.data.shape[0] - 1:
                    end_only += 1
                else:
                    middle_only += 1
            else:
                mixed += 1

            # Find any traces with duplicate offset values within the gather
            for i in range(1, offset_request.data.shape[0]):
                if offset_request.data[i] == offset_request.data[i - 1]:
                    duplicate_offsets += 1

        return dim2_size, front_only, middle_only, end_only, mixed, no_dead, duplicate_offsets


def test_gather_spacing_invalid_arg(invalid_executor):
    ex, output_vds = invalid_executor

    result = ex.run()

    assert result != 0, ex.output()
    assert "unknown --respace-gathers option" in ex.output().lower()


@pytest.mark.parametrize("respace_option", [None, "Auto", "On"])
def test_gather_spacing_with_on_variations(prestack_segy, output_vds, respace_option):
    """
    Parameterized test for the different ways to execute the importer that all result in having gather
    respacing turned On.
    """
    ex = construct_respace_executor(output_vds, prestack_segy, respace_option)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    total, leading_only, middle_only, trailing_only, mixed, no_dead, duplicate_offsets =\
        get_gathers_stats(output_vds.filename)

    assert total == 71
    assert middle_only == 23
    assert mixed == 0
    assert trailing_only == 0
    assert leading_only == 0
    assert no_dead == 48

    # There traces with duplicate offsets in this data, and the importer must preserve all those traces
    assert duplicate_offsets == 40


def test_gather_spacing_off(off_executor):
    ex, output_vds = off_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    total, leading_only, middle_only, trailing_only, mixed, no_dead, duplicate_offsets =\
        get_gathers_stats(output_vds.filename)

    assert total == 71
    assert middle_only == 0
    assert mixed == 0
    assert trailing_only == 23
    assert leading_only == 0
    assert no_dead == 48

    # There traces with duplicate offsets in this data, and the importer must preserve all those traces
    assert duplicate_offsets == 40


@pytest.mark.skip("Intended for cross-version validation; not useful for auto tests")
def test_compare_to_prestack_baseline(auto_executor, prestack_vds_baseline):
    ex, output_vds = auto_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(prestack_vds_baseline, "") as handle_baseline:
        with openvds.open(output_vds.filename, "") as handle:
            access_manager = openvds.getAccessManager(handle)
            layout = openvds.getLayout(handle)

            access_manager_baseline = openvds.getAccessManager(handle_baseline)
            layout_baseline = openvds.getLayout(handle_baseline)

            num_z = layout.getDimensionNumSamples(0)
            fold = layout.getDimensionNumSamples(1)
            num_crossline = layout.getDimensionNumSamples(2)
            num_inline = layout.getDimensionNumSamples(3)

            assert num_z == layout_baseline.getDimensionNumSamples(0)
            assert fold == layout_baseline.getDimensionNumSamples(1)
            assert num_crossline == layout_baseline.getDimensionNumSamples(2)
            assert num_inline == layout_baseline.getDimensionNumSamples(3)

            trace_channel = layout.getChannelIndex("Trace")
            trace_channel_baseline = layout_baseline.getChannelIndex("Trace")
            offset_channel = layout.getChannelIndex("Offset")
            offset_channel_baseline = layout_baseline.getChannelIndex("Offset")

            assert trace_channel > 0
            assert trace_channel_baseline > 0
            assert offset_channel > 0
            assert offset_channel_baseline > 0

            for inline_index in range(num_inline):
                for crossline_index in range(num_crossline):
                    voxel_min = (0, 0, crossline_index, inline_index, 0, 0)
                    voxel_max = (num_z, fold, crossline_index + 1, inline_index + 1, 1, 1)
                    trace_voxel_max = (1, fold, crossline_index + 1, inline_index + 1, 1, 1)

                    request = access_manager.requestVolumeSubset(voxel_min, voxel_max,
                                                                 channel=0,
                                                                 format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                 dimensionsND=openvds.DimensionsND.Dimensions_012)
                    request_baseline = access_manager_baseline.requestVolumeSubset(voxel_min, voxel_max,
                                                                                   channel=0,
                                                                                   format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                                   dimensionsND=openvds.DimensionsND.Dimensions_012)

                    trace_flag_request = access_manager.requestVolumeSubset(voxel_min, trace_voxel_max,
                                                                            channel=trace_channel,
                                                                            format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                            dimensionsND=openvds.DimensionsND.Dimensions_012)
                    trace_flag_request_baseline = access_manager_baseline.requestVolumeSubset(voxel_min,
                                                                                              trace_voxel_max,
                                                                                              channel=trace_channel_baseline,
                                                                                              format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                                              dimensionsND=openvds.DimensionsND.Dimensions_012)

                    offset_request = access_manager.requestVolumeSubset(voxel_min, trace_voxel_max,
                                                                        channel=offset_channel,
                                                                        format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                        dimensionsND=openvds.DimensionsND.Dimensions_012)
                    offset_request_baseline = access_manager_baseline.requestVolumeSubset(voxel_min, trace_voxel_max,
                                                                                          channel=offset_channel_baseline,
                                                                                          format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                                          dimensionsND=openvds.DimensionsND.Dimensions_012)

                    sample_data = request.data.reshape(fold, num_z)
                    sample_data_baseline = request_baseline.data.reshape(fold, num_z)

                    for trace_index in range(fold):
                        assert trace_flag_request.data[trace_index] == trace_flag_request_baseline.data[trace_index], \
                            f"trace index {trace_index}  xl index {crossline_index}  il index {inline_index}"
                        assert offset_request.data[trace_index] == offset_request_baseline.data[trace_index], \
                            f"trace index {trace_index}  xl index {crossline_index}  il index {inline_index}"

                        if trace_flag_request.data[trace_index] != 0:
                            for sample_index in range(num_z):
                                assert sample_data[trace_index, sample_index] == sample_data_baseline[
                                    trace_index, sample_index], \
                                    f"sample index {sample_index}  trace index {trace_index}  xl index {crossline_index}  il index {inline_index}"
