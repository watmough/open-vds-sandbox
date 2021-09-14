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

        assert layout.getChannelValueRangeMin(0) == -302681.94
        assert layout.getChannelValueRangeMax(0) == 303745.38

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

        assert layout.getChannelValueRangeMin(0) == -420799.12
        assert layout.getChannelValueRangeMax(0) == 421478.38

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
