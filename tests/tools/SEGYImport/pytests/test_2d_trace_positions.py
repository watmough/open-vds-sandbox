import os
from pathlib import Path

import pytest
import openvds
import array

from segyimport_test_config import ImportExecutor, TempVDSGuard, platform_integration_test_data_dir


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import2d_trace_positions_test")


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


def check_trace_positions(filename: str, positions_count: int):
    with openvds.open(filename, "") as handle:
        layout = openvds.getLayout(handle)

        key = openvds.KnownMetadata.tracePositions()

        assert layout.isMetadataBLOBAvailable(key.category, key.name)

        blob = layout.getMetadataBLOB(key.category, key.name)
        trace_positions = array.array("d", bytes(blob))

        assert len(trace_positions) == positions_count

        for i in range(2, len(trace_positions) - 1, 2):
            prev_x = trace_positions[i - 2]
            prev_y = trace_positions[i - 1]
            x = trace_positions[i]
            y = trace_positions[i + 1]

            assert x != 0
            assert y != 0
            assert 0 <= abs(x - prev_x) < 1000
            assert 0 <= abs(y - prev_y) < 1000


def test_trace_positions_2d_poststack(poststack_2d_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(poststack_2d_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_trace_positions(output_vds.filename, 3954)


def test_trace_positions_2d_prestack(prestack_2d_segy, output_vds):
    ex = ImportExecutor()

    ex.add_arg("--2d")
    ex.add_arg("--prestack")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(prestack_2d_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_trace_positions(output_vds.filename, 3954)


def test_trace_positions_unbinned_cdp(unbinned_cdp_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--primary-key", "ensemblenumber"])
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(unbinned_cdp_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_trace_positions(output_vds.filename, 40)


def test_trace_positions_unbinned_receiver(unbinned_receiver_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--primary-key", "receiver"])
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(unbinned_receiver_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_trace_positions(output_vds.filename, 2)


def test_trace_positions_unbinned_shot(unbinned_shot_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--primary-key", "energysourcepointnumber"])
    ex.add_args(["--vdsfile", output_vds.filename])

    # The input file has only one CDP so we need to ignore the "file only has one segment" warning
    ex.add_arg("--ignore-warnings")

    ex.add_arg(unbinned_shot_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    check_trace_positions(output_vds.filename, 3840)
