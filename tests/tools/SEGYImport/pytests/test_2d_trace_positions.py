import os
from pathlib import Path

import pytest
import openvds
import array
from typing import List, Optional
import numpy as np

from segyimport_test_config import ImportExecutor, TempVDSGuard, platform_integration_test_data_dir, segyimport_test_data_dir

@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import2d_trace_positions_test")


@pytest.fixture
def output_vds_two() -> TempVDSGuard:
    return TempVDSGuard("import2d_trace_positions_test_two")


@pytest.fixture
def poststack_2d_segy(platform_integration_test_data_dir) -> str:
    return os.path.join(platform_integration_test_data_dir, "PICEANCE-2D", "A_raw_migr_stack_FF04_03.segy")


@pytest.fixture
def prestack_2d_segy(platform_integration_test_data_dir) -> str:
    return os.path.join(platform_integration_test_data_dir, "PICEANCE-2D", "PostMigration_CDP_NMO_FF04_03_031314.sgy")


@pytest.fixture
def unbinned_cdp_segy(platform_integration_test_data_dir) -> str:
    return os.path.join(platform_integration_test_data_dir, "Teleport", "Teleport_Trim", "Unbinned_CDP", "101306.segy")


@pytest.fixture
def unbinned_receiver_segy(platform_integration_test_data_dir) -> str:
    return os.path.join(platform_integration_test_data_dir, "Teleport", "Teleport_Trim", "Unbinned_Rcvr",
                        "RLine1257_Trim_1_25000.segy")


@pytest.fixture
def unbinned_shot_segy(platform_integration_test_data_dir) -> str:
    return os.path.join(platform_integration_test_data_dir, "Teleport", "Teleport_Trim", "Unbinned_Shot", "147094.segy")


@pytest.fixture
def ensemble_gap_segy(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "PegasusBasin", "APB13-2D-PR5170-T-PSTM-FULL.2D.Full_Angle_Stack.APB13-007.segy")

def check_trace_positions(filename: str, coordinates_count: int, expected_coordinates: List,
                          expected_coordinates_tail: Optional[List] = None):
    with openvds.open(filename, "") as handle:
        layout = openvds.getLayout(handle)

        key = openvds.KnownMetadata.tracePositions()

        assert layout.isMetadataBLOBAvailable(key.category, key.name)

        blob = layout.getMetadataBLOB(key.category, key.name)
        trace_positions = array.array("d", bytes(blob))

        assert len(trace_positions) == 2 * coordinates_count

        tolerance = 0.02

        for i in range(len(expected_coordinates)):
            x = trace_positions[i * 2]
            y = trace_positions[i * 2 + 1]
            assert abs(expected_coordinates[i][0] - x) < tolerance
            assert abs(expected_coordinates[i][1] - y) < tolerance

        if expected_coordinates_tail is not None:
            offset = coordinates_count - len(expected_coordinates_tail)
            for i in range(len(expected_coordinates_tail)):
                x = trace_positions[(offset + i) * 2]
                y = trace_positions[(offset + i) * 2 + 1]
                assert abs(expected_coordinates_tail[i][0] - x) < tolerance
                assert abs(expected_coordinates_tail[i][1] - y) < tolerance


def check_2d_metadata_exists(filename: str):
    with openvds.open(filename, "") as handle:
        layout = openvds.getLayout(handle)

        key_positions = openvds.KnownMetadata.tracePositions()
        key_ensembles = openvds.KnownMetadata.ensembleNumbers()
        key_esps = openvds.KnownMetadata.energySourcePointNumbers()
        key_verts = openvds.KnownMetadata.traceVerticalOffsets()

        assert layout.isMetadataBLOBAvailable(key_positions.category, key_positions.name)
        assert layout.isMetadataBLOBAvailable(key_esps.category, key_esps.name)
        assert layout.isMetadataBLOBAvailable(key_verts.category, key_verts.name)

        # this metadata is NOT present when the primary axis is CDP (the default)
        assert not layout.isMetadataBLOBAvailable(key_ensembles.category, key_ensembles.name)


def test_trace_positions_2d_poststack(poststack_2d_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(poststack_2d_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    expected_positions = [
        (374715.94, 224982.33),
        (374716.25, 224974.09),
        (374716.25, 224965.56),
        (374716.25, 224957.33),
        (374716.53, 224948.80),
        (374716.53, 224940.56),
        (374716.53, 224932.03),
        (374716.84, 224923.81),
        (374716.84, 224915.27),
        (374716.84, 224907.05),
        (374717.16, 224898.50),
        (374717.16, 224890.28),
        (374717.16, 224881.75),
        (374717.49, 224873.52),
        (374717.49, 224864.98),
        (374717.49, 224856.75),
        (374717.75, 224848.22),
        (374717.75, 224839.98),
        (374718.06, 224831.45),
        (374718.06, 224823.22)
    ]
    expected_positions_tail = [
        (368471.19, 210092.55),
        (368464.50, 210087.67),
        (368457.78, 210082.48),
        (368450.78, 210077.61),
        (368444.06, 210072.73),
        (368437.06, 210067.86),
        (368430.34, 210063.28),
        (368423.34, 210058.41),
        (368416.63, 210053.53),
        (368409.63, 210048.95),
        (368402.63, 210044.08),
        (368395.91, 210039.50),
        (368388.91, 210034.63),
        (368381.88, 210030.06),
        (368375.19, 210025.19),
        (368368.16, 210020.31),
        (368361.16, 210015.73),
        (368354.47, 210010.86),
        (368347.44, 210006.28),
        (368340.44, 210001.41)
    ]

    check_trace_positions(output_vds.filename, 1977, expected_positions, expected_positions_tail)
    check_2d_metadata_exists(output_vds.filename)


def test_trace_positions_2d_prestack(prestack_2d_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_arg("--prestack")
    ex.add_args(["--header-field", "ensemblexcoordinate=73:4"])
    ex.add_args(["--header-field", "ensembleycoordinate=77:4"])
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(prestack_2d_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    expected_coordinates = [
        (374715.02, 224982.33),
        (374714.72, 224973.79),
        (374715.63, 224965.26),
        (374715.02, 224956.73),
        (374715.94, 224948.19),
        (374715.63, 224939.66),
        (374716.85, 224931.12),
        (374717.16, 224922.59),
        (374718.07, 224914.05),
        (374718.38, 224905.52),
        (374719.90, 224896.98),
        (374719.90, 224896.98),
        (374719.90, 224896.98),
        (374719.90, 224896.98),
        (374720.82, 224863.15),
        (374722.64, 224854.31),
        (374723.25, 224845.78),
        (374722.95, 224837.55),
        (374721.42, 224829.01),
        (374721.42, 224829.01),
    ]
    expected_coordinates_tail = [
        (368576.66, 210135.52),
        (368576.66, 210135.52),
        (368576.66, 210135.52),
        (368576.66, 210135.52),
        (368576.66, 210135.52),
        (368576.66, 210135.52),
        (368576.66, 210135.52),
        (368576.66, 210135.52),
        (368425.48, 210036.77),
        (368418.47, 210032.19),
        (368410.54, 210029.45),
        (368401.70, 210028.23),
        (368394.39, 210024.27),
        (368385.55, 210023.96),
        (368377.32, 210021.83),
        (368377.32, 210021.83),
        (368377.32, 210021.83),
        (368352.93, 210014.21),
        (368346.23, 210008.72),
        (368339.22, 210004.15),
    ]

    check_trace_positions(output_vds.filename, 1977, expected_coordinates, expected_coordinates_tail)
    check_2d_metadata_exists(output_vds.filename)


def test_trace_positions_unbinned_cdp(unbinned_cdp_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--primary-key", "ensemblenumber"])
    ex.add_args(["--header-field", "ensemblexcoordinate=189:4"])
    ex.add_args(["--header-field", "ensembleycoordinate=193:4"])
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(unbinned_cdp_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    expected_positions = [
        (760825.0, 2216373.0),
        (760835.0, 2216381.0),
        (760845.0, 2216389.0),
        (760854.0, 2216396.0),
        (760864.0, 2216404.0),
        (760874.0, 2216412.0),
        (760884.0, 2216420.0),
        (760894.0, 2216428.0),
        (760903.0, 2216435.0),
        (760913.0, 2216443.0)
    ]

    check_trace_positions(output_vds.filename, len(expected_positions), expected_positions)

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        key_ensemble = openvds.KnownMetadata.ensembleNumbers()

        assert layout.isMetadataBLOBAvailable(key_ensemble.category, key_ensemble.name)


def test_trace_positions_unbinned_receiver(unbinned_receiver_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--primary-key", "receiver"])
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(unbinned_receiver_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # The group X/Y coordinate header fields for this data all have the same values.
    # (Perhaps we're misinterpreting the header field locations.)
    expected_x = 510840.3
    expected_y = 639536.9

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        key = openvds.KnownMetadata.tracePositions()

        assert layout.isMetadataBLOBAvailable(key.category, key.name)

        blob = layout.getMetadataBLOB(key.category, key.name)
        trace_positions = array.array("d", bytes(blob))

        position_pair_count = layout.getDimensionNumSamples(2)

        assert len(trace_positions) == 2 * position_pair_count

        for i in range(position_pair_count):
            x = trace_positions[i * 2]
            y = trace_positions[i * 2 + 1]
            assert abs(expected_x - x) < 0.01
            assert abs(expected_y - y) < 0.01


def test_trace_positions_unbinned_shot(unbinned_shot_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--primary-key", "energysourcepointnumber"])
    ex.add_args(["--vdsfile", output_vds.filename])
    ex.add_args(["--header-field", "ensemblexcoordinate=189:4"])
    ex.add_args(["--header-field", "ensembleycoordinate=193:4"])

    # The input file has only one CDP so we need to ignore the "file only has one segment" warning
    ex.add_arg("--ignore-warnings")

    ex.add_arg(unbinned_shot_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    expected_positions = [(665198.0, 2581094.0)]

    check_trace_positions(output_vds.filename, 1, expected_positions)

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        key_esp = openvds.KnownMetadata.energySourcePointNumbers()

        assert layout.isMetadataBLOBAvailable(key_esp.category, key_esp.name)


def test_trace_positions_poststack_2d_with_ensemble_number_gap(ensemble_gap_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(ensemble_gap_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    expected_positions = [
        (1869969.0, 5347081.0),
        (1869957.0, 5347077.0),
        (1869946.0, 5347072.0),
        (1869934.0, 5347068.0),
        (1869923.0, 5347063.0),
        (1869911.0, 5347059.0),
        (1869899.0, 5347054.0),
        (1869888.0, 5347050.0),
        (1869876.0, 5347045.0),
        (1869864.0, 5347041.0),
        (1869853.0, 5347036.0),
        (1869841.0, 5347032.0),
        (1869830.0, 5347027.0),
        (1869818.0, 5347023.0),
        (1869806.0, 5347018.0),
        (1869795.0, 5347014.0),
        (1869783.0, 5347009.0),
        (1869771.0, 5347005.0),
        (1869760.0, 5347000.0),
        (1869748.0, 5346996.0)
    ]
    expected_positions_tail = [
        (1742312.0, 5297580.0),
        (1742300.0, 5297575.0),
        (1742288.0, 5297571.0),
        (1742276.0, 5297566.0),
        (1742264.0, 5297562.0),
        (1742251.0, 5297557.0),
        (1742239.0, 5297553.0),
        (1742227.0, 5297548.0),
        (1742215.0, 5297544.0),
        (1742203.0, 5297540.0),
        (1742190.0, 5297535.0),
        (1742178.0, 5297531.0),
        (1742167.0, 5297527.0),
        (1742155.0, 5297523.0),
        (1742143.0, 5297519.0),
        (1742131.0, 5297515.0),
        (1742120.0, 5297511.0),
        (1742108.0, 5297507.0),
        (1742096.0, 5297503.0),
        (1742084.0, 5297499.0)
    ]

    check_trace_positions(output_vds.filename, 10975, expected_positions, expected_positions_tail)


def test_ensemble_gap_axis_size_unitless_axis(ensemble_gap_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_arg("--2d-index-axis")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(ensemble_gap_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        # 10974 is actual number of traces in the SEGY file
        assert layout.getDimensionNumSamples(1) == 10974


def test_ensemble_gap_axis_size_cdp_axis(ensemble_gap_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(ensemble_gap_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        # SEGY file has 10974 traces. Using CDP for the primary axis results in SEGYImport inserting a dead
        # trace where the SEGY data has a gap in the sequence of CDP numbers. Thus the VDS has 10975 traces.
        assert layout.getDimensionNumSamples(1) == 10975


def test_compare_axis_styles(ensemble_gap_segy, output_vds, output_vds_two):
    # Import using the default axis annotation
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_arg("--2d-index-axis")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(ensemble_gap_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    # Import using CDP axis annotation
    ex2 = ImportExecutor()

    ex2.add_arg("--2d")
    # CDP is the default, no arg required
    ex2.add_args(["--vdsfile", output_vds_two.filename])

    ex2.add_arg(ensemble_gap_segy)

    result = ex2.run()

    assert result == 0, ex2.output()
    assert Path(output_vds_two.filename).exists()

    with openvds.open(output_vds.filename, "") as handle_default,\
            openvds.open(output_vds_two.filename, "") as handle_cdp:
        access_default = openvds.getAccessManager(handle_default)
        access_cdp = openvds.getAccessManager(handle_cdp)
        layout_default = openvds.getLayout(handle_default)
        layout_cdp = openvds.getLayout(handle_cdp)

        assert layout_default.getDimensionNumSamples(0) == layout_cdp.getDimensionNumSamples(0)

        trace_length = layout_default.getDimensionNumSamples(0)
        trace_count_default = layout_default.getDimensionNumSamples(1)
        trace_count_cdp = layout_cdp.getDimensionNumSamples(1)

        request_default = access_default.requestVolumeSubset((0, 0, 0, 0, 0, 0),
                                                             (trace_length, trace_count_default, 1, 1, 1, 1),
                                                             channel=0,
                                                             format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                             dimensionsND=openvds.DimensionsND.Dimensions_01)
        request_cdp = access_cdp.requestVolumeSubset((0, 0, 0, 0, 0, 0),
                                                         (trace_length, trace_count_cdp, 1, 1, 1, 1),
                                                         channel=0,
                                                         format=openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                         dimensionsND=openvds.DimensionsND.Dimensions_01)

        data_default = request_default.data.reshape(trace_count_default, trace_length)
        data_cdp = request_cdp.data.reshape(trace_count_cdp, trace_length)

        for i in range(4864):
            trace_default = data_default[i, :]
            trace_cdp = data_cdp[i, :]
            np.testing.assert_array_equal(trace_default, trace_cdp, err_msg=f"Trace mismatch at index {i}")

        dead_trace = data_cdp[4864, :]
        np.testing.assert_array_equal(dead_trace, np.zeros((trace_length, ), dtype=np.float32),
                                      err_msg="Dead trace should be all zeros")

        for i in range(4864, trace_count_default):
            trace_default = data_default[i, :]
            trace_cdp = data_cdp[i + 1, :]
            np.testing.assert_array_equal(trace_default, trace_cdp, err_msg=f"Trace mismatch at index {i}")
