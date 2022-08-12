import os
from pathlib import Path
from typing import List

import pytest
import openvds

from segyimport_test_config import segyimport_test_data_dir, ImportExecutor, TempScanFileGuard, TempVDSGuard


@pytest.fixture
def multi_file_input_base_name() -> str:
    return "ST0202R08_Gather_Time_pt"


@pytest.fixture
def multi_file_input_parts_count() -> int:
    # the multi-file SEGY consists of this many files
    return 11


@pytest.fixture
def multi_file_input_dir(segyimport_test_data_dir) -> str:
    return os.path.join(segyimport_test_data_dir, "MultiFile", "ST0202R08_Gather_Time")


@pytest.fixture
def multi_file_input_glob(multi_file_input_dir) -> str:
    return os.path.join(multi_file_input_dir, "ST0202R08_Gather_Time_pt??.segy")


@pytest.fixture
def multi_file_input_files(multi_file_input_parts_count, multi_file_input_base_name, multi_file_input_dir) -> List[str]:
    filenames = []
    for i in range(1, multi_file_input_parts_count + 1):
        input_name = f"{multi_file_input_base_name}{i:02}.segy"
        filenames.append(os.path.join(multi_file_input_dir, input_name))
    return filenames


@pytest.fixture
def multi_file_scan_file_info_files(multi_file_input_parts_count, multi_file_input_base_name) -> List[TempScanFileGuard]:
    """File info filenames to be output when using --scan"""
    guards = []
    for i in range(1, multi_file_input_parts_count + 1):
        file_info_base_name = f"{multi_file_input_base_name}{i:02}.segy"
        guards.append(TempScanFileGuard(file_info_base_name))
    return guards


@pytest.fixture
def multi_file_input_file_info_files(multi_file_input_parts_count, multi_file_input_base_name, multi_file_input_dir) -> List[str]:
    """File info filenames to be used when importing"""
    filenames = []
    for i in range(1, multi_file_input_parts_count + 1):
        file_info_name = f"{multi_file_input_base_name}{i:02}.segy.scan.json"
        filenames.append(os.path.join(multi_file_input_dir, file_info_name))
    return filenames


def test_multi_file_scan_one_file_info(multi_file_input_glob):
    """
    Tests --scan with multiple input SEGY files, but only one file info file specified.
    """
    file_info_guard = TempScanFileGuard("multi_single_test")

    ex = ImportExecutor()
    ex.add_args(["--prestack", "--scan", "--file-info", file_info_guard.filename, multi_file_input_glob])
    result = ex.run()

    # import should have failed
    assert result > 0, ex.output()

    # error message should contain this
    assert "Different number of input SEG-Y file names and file-info file names".lower() in ex.output().lower()


def test_multi_file_scan(multi_file_input_files, multi_file_scan_file_info_files):
    ex = ImportExecutor()
    ex.add_args(["--prestack", "--scan"])

    for scan_file_guard in multi_file_scan_file_info_files:
        ex.add_args(["--file-info", scan_file_guard.filename])

        # ensure the output file doesn't exist
        if Path(scan_file_guard.filename).exists():
            os.remove(scan_file_guard.filename)

    ex.add_args(multi_file_input_files)

    result = ex.run()

    # import should have succeeded
    assert result == 0, ex.output()

    # output files should exist
    for filename in multi_file_scan_file_info_files:
        assert Path(scan_file_guard.filename).exists()


def test_multi_file_import_with_file_infos(multi_file_input_files, multi_file_input_file_info_files):
    ex = ImportExecutor()
    ex.add_arg("--prestack")

    vds_guard = TempVDSGuard("import_test")
    ex.add_args(["--vdsfile", vds_guard.filename])

    for filename in multi_file_input_file_info_files:
        ex.add_args(["--file-info", filename])

    ex.add_args(multi_file_input_files)

    result = ex.run()

    # import should have succeeded
    assert result == 0, ex.output()

    # output file should exist
    assert Path(vds_guard.filename).exists()

    # check dimensions of VDS
    with openvds.open(vds_guard.filename, "") as handle:
        layout = openvds.getLayout(handle)
        assert layout.dimensionality == 4
        assert layout.numSamples[0] == 851
        assert layout.numSamples[1] == 100
        assert layout.numSamples[2] == 71
        assert layout.numSamples[3] == 21
