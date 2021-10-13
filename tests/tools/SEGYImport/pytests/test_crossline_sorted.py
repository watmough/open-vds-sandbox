from pathlib import Path
from typing import List, Tuple

import openvds
import pytest
import os

from segyimport_test_config import ImportExecutor, TempVDSGuard, segyimport_test_data_dir, teleport_test_data_dir, \
    platform_integration_test_data_dir


def construct_crossline_executor(segy_filename: str, output_vds: TempVDSGuard,
                                 additional_args: List[str] = None) -> ImportExecutor:
    ex = ImportExecutor()

    ex.add_args(["--primary-key", "CrosslineNumber"])
    ex.add_args(["--secondary-key", "InlineNumber"])
    ex.add_args(["--vdsfile", output_vds.filename])
    if additional_args:
        ex.add_args(additional_args)
    ex.add_arg(segy_filename)

    return ex


@pytest.fixture
def crossline_output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_crossline_test")


@pytest.fixture
def conventional_output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_conventional_test")


@pytest.fixture
def teleport_conventional_segy(teleport_test_data_dir) -> str:
    return os.path.join(teleport_test_data_dir, "Teleport_Trim", "3D_Stack", "ST0202R08_TIME.segy")


@pytest.fixture
def teleport_crossline_sorted_segy(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "CrosslineSorted", "ST0202R08_TIME_crossline_sorted.segy")


@pytest.fixture
def teleport_crossline_executor(teleport_crossline_sorted_segy, crossline_output_vds)\
        -> Tuple[ImportExecutor, TempVDSGuard]:
    ex = construct_crossline_executor(teleport_crossline_sorted_segy, crossline_output_vds)
    return ex, crossline_output_vds


@pytest.fixture
def teleport_conventional_executor(teleport_conventional_segy, conventional_output_vds)\
        -> Tuple[ImportExecutor, TempVDSGuard]:
    ex = ImportExecutor()

    ex.add_args(["--vdsfile", conventional_output_vds.filename])
    ex.add_arg(teleport_conventional_segy)

    return ex, conventional_output_vds


def get_scs_metadata_vectors(layout: openvds.VolumeDataLayout) -> Tuple[Tuple[float, float], Tuple[float, float],
                                                                        Tuple[float, float]]:
    """Convenience method to retrieve survey coordinate system vectors from VDS metadata"""

    origin = layout.getMetadataDoubleVector2(openvds.KnownMetadata.surveyCoordinateSystemOrigin().category,
                                             openvds.KnownMetadata.surveyCoordinateSystemOrigin().name)
    inline_spacing = layout.getMetadataDoubleVector2(
        openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().category,
        openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().name)
    crossline_spacing = layout.getMetadataDoubleVector2(
        openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().category,
        openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().name)

    return origin, inline_spacing, crossline_spacing


def test_coordinate_system(teleport_crossline_executor):
    """
    Examine the survey coordinate system metadata for a crossline-sorted SEGY.

    Note that these survey coordinate system values should be the same as the values examined in
    test_coordinate_system_comparison, but we'll check explicit values here to ensure that the importer
    is producing expected values (and not just two sets of incorrect but identical values).
    """
    ex, output_vds = teleport_crossline_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        origin, inline_spacing, crossline_spacing = get_scs_metadata_vectors(layout)

        assert origin[0] == pytest.approx(431990.979266)
        assert origin[1] == pytest.approx(6348549.7101)
        assert inline_spacing[0] == pytest.approx(3.021094)
        assert inline_spacing[1] == pytest.approx(12.125)
        assert crossline_spacing[0] == pytest.approx(-12.128862)
        assert crossline_spacing[1] == pytest.approx(3.022056)


def test_coordinate_system_comparison(teleport_crossline_executor, teleport_conventional_executor):
    """
    Compare the survey coordinate system metadata for a crossline-sorted SEGY with the same data stored in a
    conventionally-sorted SEGY.
    """
    xl_ex, xl_output_vds = teleport_crossline_executor

    result = xl_ex.run()

    assert result == 0, xl_ex.output()
    assert Path(xl_output_vds.filename).exists()

    conv_ex, conv_output_vds = teleport_crossline_executor

    result = conv_ex.run()

    assert result == 0, conv_ex.output()
    assert Path(conv_output_vds.filename).exists()

    with openvds.open(xl_output_vds.filename, "") as xl_handle,\
            openvds.open(conv_output_vds.filename, "") as conv_handle:
        xl_layout = openvds.getLayout(xl_handle)
        conv_layout = openvds.getLayout(conv_handle)

        xl_origin, xl_inline, xl_crossline = get_scs_metadata_vectors(xl_layout)
        conv_origin, conv_inline, conv_crossline = get_scs_metadata_vectors(conv_layout)

        assert xl_origin == conv_origin
        assert xl_inline == conv_inline
        assert xl_crossline == conv_crossline


def test_samples_comparison(teleport_crossline_executor, teleport_conventional_executor):
    xl_ex, xl_output_vds = teleport_crossline_executor

    result = xl_ex.run()

    assert result == 0, xl_ex.output()
    assert Path(xl_output_vds.filename).exists()

    conv_ex, conv_vds = teleport_conventional_executor

    result = conv_ex.run()

    assert result == 0, conv_ex.output()
    assert Path(conv_vds.filename).exists()

    with openvds.open(xl_output_vds.filename, "") as handle_xl, openvds.open(conv_vds.filename, "") as handle_conv:
        access_manager_xl = openvds.getAccessManager(handle_xl)
        layout_xl = openvds.getLayout(handle_xl)

        access_manager_conv = openvds.getAccessManager(handle_conv)
        layout_conv = openvds.getLayout(handle_conv)

        num_z = layout_xl.getDimensionNumSamples(0)
        num_crossline = layout_xl.getDimensionNumSamples(1)
        num_inline = layout_xl.getDimensionNumSamples(2)

        assert num_z == layout_conv.getDimensionNumSamples(0)
        assert num_crossline == layout_conv.getDimensionNumSamples(1)
        assert num_inline == layout_conv.getDimensionNumSamples(2)

        for inline_index in range(num_inline):
            voxel_min = (0, 0, inline_index, 0, 0, 0)
            voxel_max = (num_z, num_crossline, inline_index + 1, 1, 1)

            request = access_manager_xl.requestVolumeSubset(voxel_min, voxel_max,
                                                            channel=0,
                                                            format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                            dimensionsND=openvds.DimensionsND.Dimensions_012)
            request_conv = access_manager_conv.requestVolumeSubset(voxel_min, voxel_max,
                                                                   channel=0,
                                                                   format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                                   dimensionsND=openvds.DimensionsND.Dimensions_012)

            sample_data = request.data.reshape(num_crossline, num_z)
            sample_data_conv = request_conv.data.reshape(num_crossline, num_z)

            for crossline_index in range(num_crossline):
                for sample_index in range(num_z):
                    assert sample_data[crossline_index, sample_index] == sample_data_conv[crossline_index, sample_index],\
                        f"sample index {sample_index}  xl index {crossline_index}  il index {inline_index}"


poststack_axes = [
    (1477, "Sample", "ms", 0.0, 5904.0),
    (101, "Crossline", "unitless", 2000.0, 2100.0),
    (101, "Inline", "unitless", 1000.0, 1100.0),
    ()
]

prestack_axes = [
    (1477, "Sample", "ms", 0.0, 5904.0),
    (60, "Trace (offset)", "unitless", 1.0, 60.0),
    (10, "Crossline", "unitless", 2031.0, 2040.0),
    (101, "Inline", "unitless", 1000.0, 1100.0)
]

poststack_survey_vectors = [
    (76290.81610169, 949398.18641265),
    (-15.5, 19.609375),
    (19.60459195, 15.49621929)
]

prestack_survey_vectors = [
    (530620.06457521, 5789005.97294581),
    (-15.51, 19.625),
    (19.63167672, 15.51527674)
]

poststack_segy_filename = os.path.join(platform_integration_test_data_dir(), "ImporterTests",
                                       "poststack_Xl-inc_In-inc.segy")
prestack_segy_filename = os.path.join(platform_integration_test_data_dir(), "ImporterTests",
                                      "prestack_Xl-inc_In-inc_2031-2040.segy")


@pytest.mark.parametrize("test_params", [(poststack_segy_filename, 3, poststack_axes, poststack_survey_vectors),
                         (prestack_segy_filename, 4, prestack_axes, prestack_survey_vectors)])
def test_dimensions_and_produce_status(crossline_output_vds,
                                       test_params: Tuple[str, int, List[Tuple[int, str, str, float, float]],
                                                          List[Tuple[float, float]]]):
    segy_filename, dimensions, axes, scs_vectors = test_params

    additional_args = [
        "--header-field", "InlineNumber=17:4",
        "--header-field", "CrosslineNumber=17:4",
        "--header-field", "EnsembleXCoordinateByteLocation=73:4",
        "--header-field", "EnsembleYCoordinateByteLocation=77:4"
    ]
    if dimensions == 4:
        additional_args.append("--prestack")

    ex = construct_crossline_executor(segy_filename, crossline_output_vds, additional_args)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(crossline_output_vds.filename).exists()

    with openvds.open(crossline_output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)
        access_manager = openvds.getAccessManager(handle)

        sample_channel_name = layout.getChannelName(0)

        # check produce status

        if dimensions == 4:
            # Dim_013 should be available/remapped
            assert access_manager.getVDSProduceStatus(
                openvds.DimensionsND.Dimensions_013) != openvds.VDSProduceStatus.Unavailable

            # These dimensions (and others) should be unavailable
            assert access_manager.getVDSProduceStatus(
                openvds.DimensionsND.Dimensions_01) == openvds.VDSProduceStatus.Unavailable
            assert access_manager.getVDSProduceStatus(
                openvds.DimensionsND.Dimensions_012) == openvds.VDSProduceStatus.Unavailable
        elif dimensions == 3:
            # Dim_012 should be available/remapped
            assert access_manager.getVDSProduceStatus(
                openvds.DimensionsND.Dimensions_012) != openvds.VDSProduceStatus.Unavailable

            # These dimensions (and others) should be unavailable
            assert access_manager.getVDSProduceStatus(
                openvds.DimensionsND.Dimensions_01) == openvds.VDSProduceStatus.Unavailable
        else:
            assert False, f"Invalid number of VDS dimensions: {dimensions}"

        # check axes

        for dim in range(dimensions):
            descriptor = layout.getAxisDescriptor(dim)
            axis_props = axes[dim]
            assert descriptor.getNumSamples() == axis_props[0]
            assert descriptor.getName() == axis_props[1]
            assert descriptor.getUnit() == axis_props[2]
            assert descriptor.getCoordinateMin() == axis_props[3]
            assert descriptor.getCoordinateMax() == axis_props[4]

        # check survey coordinate system vectors

        origin, inline_spacing, crossline_spacing = get_scs_metadata_vectors(layout)
        assert origin[0] == scs_vectors[0][0]
        assert origin[1] == scs_vectors[0][1]
        assert inline_spacing[0] == scs_vectors[1][0]
        assert inline_spacing[1] == scs_vectors[1][1]
        assert crossline_spacing[0] == scs_vectors[2][0]
        assert crossline_spacing[1] == scs_vectors[2][1]
