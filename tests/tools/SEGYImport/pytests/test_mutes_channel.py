import os
from pathlib import Path
from typing import Tuple

import pytest
import openvds

from segyimport_test_config import ImportExecutor, TempVDSGuard, segyimport_test_data_dir


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_mutes_test")


@pytest.fixture
def mutes_segy(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "Mutes",
                        "ST0202R08_PS_PrSDM_CIP_gathers_in_PP_Time_modified_subset.segy")


@pytest.fixture
def mutes_executor(mutes_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Fixture to setup an ImportExecutor with common options for SEGY with mutes"""
    ex = ImportExecutor()

    ex.add_arg("--mute")
    ex.add_arg("--prestack")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(mutes_segy)

    return ex, output_vds


def test_mutes_channel_descriptor(mutes_executor):
    ex, output_vds = mutes_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        mutes_channel = layout.getChannelIndex("Mute")
        assert mutes_channel > 0


def test_mutes_read(mutes_executor):
    ex, output_vds = mutes_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        access_manager = openvds.getAccessManager(handle)
        layout = openvds.getLayout(handle)

        trace_channel = layout.getChannelIndex("Trace")
        assert trace_channel > 0

        mutes_channel = layout.getChannelIndex("Mute")
        assert mutes_channel > 0

        inline_index = layout.getDimensionNumSamples(3) // 2
        dim2_size = layout.getDimensionNumSamples(2)
        dim1_size = layout.getDimensionNumSamples(1)
        dim0_size = layout.getDimensionNumSamples(0)

        for crossline_index in range(dim2_size):
            # read one gather of samples and trace flags and mutes

            voxel_min = (0, 0, crossline_index, inline_index, 0, 0)
            # voxel_max = (dim0_size, dim1_size, crossline_index + 1, inline_index + 1, 1, 1)
            trace_voxel_max = (1, dim1_size, crossline_index + 1, inline_index + 1, 1, 1)

            # request = access_manager.requestVolumeSubset(voxel_min,
            #                                              voxel_max,
            #                                              channel=0,
            #                                              format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
            #                                              dimensionsND=openvds.DimensionsND.Dimensions_012)
            trace_flag_request = access_manager.requestVolumeSubset(voxel_min,
                                                                    trace_voxel_max,
                                                                    channel=trace_channel,
                                                                    format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                    dimensionsND=openvds.DimensionsND.Dimensions_012)
            mutes_request = access_manager.requestVolumeSubset(voxel_min,
                                                               trace_voxel_max,
                                                               channel=mutes_channel,
                                                               format=openvds.VolumeDataChannelDescriptor.Format.Format_U16,
                                                               dimensionsND=openvds.DimensionsND.Dimensions_012)

            for trace_index in range(dim1_size):
                if trace_flag_request.data[trace_index] > 0:
                    assert 0 <= mutes_request.data[trace_index * 2] < mutes_request.data[trace_index * 2 + 1]
