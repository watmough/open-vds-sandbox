import os
from pathlib import Path

import pytest
import openvds

from segyimport_test_config import test_data_dir, ImportExecutor, TempVDSGuard


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import2d_test")


@pytest.fixture
def poststack_2d_segy() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "PICEANCE-2D",
                        "A_raw_migr_stack_FF04_03.segy")


@pytest.fixture
def prestack_2d_segy() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "PICEANCE-2D",
                        "PostMigration_CDP_NMO_FF04_03_031314.sgy")


def test_2d_poststack_volume_info(poststack_2d_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(poststack_2d_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        assert layout.getDimensionality() == 2
        assert layout.getChannelCount() == 3
        assert layout.getChannelName(0) == "Amplitude"
        assert layout.getChannelIndex("Trace") > 0
        assert layout.getChannelIndex("SEGYTraceHeader") > 0

        descriptor = layout.getAxisDescriptor(0)
        assert descriptor.name == "Sample"
        assert descriptor.unit == "ms"
        assert descriptor.coordinateMin == 0.0
        assert descriptor.coordinateMax == 5000.0
        assert descriptor.coordinateStep == 2.0

        descriptor = layout.getAxisDescriptor(1)
        assert descriptor.name == "CDP"
        assert descriptor.unit == "unitless"
        assert descriptor.coordinateMin == 1.0
        assert descriptor.coordinateMax == 1977.0
        assert descriptor.coordinateStep == 1.0

        assert layout.getChannelValueRangeMin(0) == -302681.9375
        assert layout.getChannelValueRangeMax(0) == 303745.375

        # check lattice metadata is NOT there
        assert not layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().category,
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().name)
        assert not layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().name)
        assert not layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().name)

        # check 2D trace info metadata is present
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.tracePositions().category,
                                              openvds.KnownMetadata.tracePositions().name)
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.traceVerticalOffsets().category,
                                              openvds.KnownMetadata.traceVerticalOffsets().name)
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.energySourcePointNumbers().category,
                                              openvds.KnownMetadata.energySourcePointNumbers().name)
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.ensembleNumbers().category,
                                              openvds.KnownMetadata.ensembleNumbers().name)


def test_2d_poststack_read(poststack_2d_segy, output_vds):
    # run importer
    # open VDS
    # read VDS (entire thing at once)
    # check that each trace is non-zero

    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(poststack_2d_segy)

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

        request = access_manager.requestVolumeSubset((0, 0, 0, 0, 0, 0),
                                                     (dim0_size, dim1_size, 1, 1, 1, 1),
                                                     channel=0,
                                                     format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                     dimensionsND=openvds.DimensionsND.Dimensions_01)
        trace_flag_request = access_manager.requestVolumeSubset((0, 0, 0, 0, 0, 0),
                                                                (1, dim1_size, 1, 1, 1, 1),
                                                                channel=trace_channel,
                                                                format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                dimensionsND=openvds.DimensionsND.Dimensions_01)

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


def test_2d_prestack_volume_info(prestack_2d_segy, output_vds):
    # run importer
    # open VDS
    # check dimensionality (3), channel count (4), channel names
    # check 3 axis descriptors
    # check sample value range
    # check lattice metadata is NOT there
    # check 2D trace info metadata

    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_arg("--prestack")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(prestack_2d_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        assert layout.getDimensionality() == 3
        assert layout.getChannelCount() == 4
        assert layout.getChannelName(0) == "Amplitude"
        assert layout.getChannelIndex("Offset") > 0
        assert layout.getChannelIndex("Trace") > 0
        assert layout.getChannelIndex("SEGYTraceHeader") > 0

        descriptor = layout.getAxisDescriptor(0)
        assert descriptor.name == "Sample"
        assert descriptor.unit == "ms"
        assert descriptor.coordinateMin == 0.0
        assert descriptor.coordinateMax == 5000.0
        assert descriptor.coordinateStep == 2.0

        descriptor = layout.getAxisDescriptor(1)
        assert descriptor.name == "Trace (offset)"
        assert descriptor.unit == "unitless"
        assert descriptor.coordinateMin == 1.0
        assert descriptor.coordinateMax == 64.0
        assert descriptor.coordinateStep == 1.0

        descriptor = layout.getAxisDescriptor(2)
        assert descriptor.name == "CDP"
        assert descriptor.unit == "unitless"
        assert descriptor.coordinateMin == 1.0
        assert descriptor.coordinateMax == 1977.0
        assert descriptor.coordinateStep == 1.0

        assert layout.getChannelValueRangeMin(0) == -420799.125
        assert layout.getChannelValueRangeMax(0) == 421478.375

        # check lattice metadata is NOT there
        assert not layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().category,
            openvds.KnownMetadata.surveyCoordinateSystemOrigin().name)
        assert not layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().name)
        assert not layout.isMetadataDoubleVector2Available(
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().category,
            openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().name)

        # check 2D trace info metadata is present
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.tracePositions().category,
                                              openvds.KnownMetadata.tracePositions().name)
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.traceVerticalOffsets().category,
                                              openvds.KnownMetadata.traceVerticalOffsets().name)
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.energySourcePointNumbers().category,
                                              openvds.KnownMetadata.energySourcePointNumbers().name)
        assert layout.isMetadataBLOBAvailable(openvds.KnownMetadata.ensembleNumbers().category,
                                              openvds.KnownMetadata.ensembleNumbers().name)


def test_2d_prestack_read(prestack_2d_segy, output_vds):
    # run importer
    # open VDS
    # read VDS (entire thing at once)
    # check that each trace is non-zero

    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_arg("--prestack")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(prestack_2d_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)
        access_manager = openvds.getAccessManager(handle)
        dim0_size = layout.getDimensionNumSamples(0)
        dim1_size = layout.getDimensionNumSamples(1)
        dim2_size = layout.getDimensionNumSamples(2)

        trace_channel = layout.getChannelIndex("Trace")
        assert trace_channel > 0
        offset_channel = layout.getChannelIndex("Offset")
        assert offset_channel > 0

        request = access_manager.requestVolumeSubset((0, 0, 0, 0, 0, 0),
                                                     (dim0_size, dim1_size, dim2_size, 1, 1, 1),
                                                     channel=0,
                                                     format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                     dimensionsND=openvds.DimensionsND.Dimensions_012)
        offset_request = access_manager.requestVolumeSubset((0, 0, 0, 0, 0, 0),
                                                            (1, dim1_size, dim2_size, 1, 1, 1),
                                                            channel=offset_channel,
                                                            format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                            dimensionsND=openvds.DimensionsND.Dimensions_012)
        trace_flag_request = access_manager.requestVolumeSubset((0, 0, 0, 0, 0, 0),
                                                                (1, dim1_size, dim2_size, 1, 1, 1),
                                                                channel=trace_channel,
                                                                format=openvds.VolumeDataChannelDescriptor.Format.Format_U8,
                                                                dimensionsND=openvds.DimensionsND.Dimensions_012)

        data = request.data.reshape(dim2_size, dim1_size, dim0_size)
        offset_data = offset_request.data.reshape(dim2_size, dim1_size)
        trace_flag_data = trace_flag_request.data.reshape(dim2_size, dim1_size)

        for dim2 in range(dim2_size):
            for dim1 in range(dim1_size):
                trace_flag = trace_flag_data[dim2, dim1]
                trace_offset = offset_data[dim2, dim1]
                ensemble_number = layout.getAxisDescriptor(2).sampleIndexToCoordinate(dim2)

                total = 0
                for dim0 in range(dim0_size):
                    total += abs(data[dim2, dim1, dim0])

                message = f"trace at dim1 {dim1} dim2 {dim2} offset {trace_offset} CDP {ensemble_number}"

                if trace_flag == 0:
                    assert total == 0.0, f"dead {message}"
                else:
                    assert total > 0.0, message
