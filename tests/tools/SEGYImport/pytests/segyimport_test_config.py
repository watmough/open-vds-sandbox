"""
Global configuration information for SEGYImport pytests
"""
import os
import subprocess
import tempfile
import weakref
import pytest
from typing import List

test_data_dir = os.getenv("SEGYIMPORT_TEST_DATA_DIR")
if not test_data_dir:
    raise EnvironmentError("SEGYIMPORT_TEST_DATA_DIR environment variable is not set")

test_data_dir_tt = os.getenv("SEGYIMPORT_TEST_DATA_DIR_TT")
if not test_data_dir_tt:
    raise EnvironmentError("SEGYIMPORT_TEST_DATA_DIR_TT environment variable is not set")

test_data_dir_hh = os.getenv("SEGYIMPORT_TEST_DATA_DIR_HH")
if not test_data_dir_hh:
    raise EnvironmentError("SEGYIMPORT_TEST_DATA_DIR_HH environment variable is not set")

_temp_dir = None


class ImportExecutor:
    def __init__(self):
        import_exe = os.getenv("SEGYIMPORT_EXECUTABLE")
        if not import_exe:
            raise EnvironmentError("SEGYIMPORT_EXECUTABLE environment variable is not set")
        self.args = [import_exe]
        self.run_result = None

    def add_arg(self, more_arg: str):
        self.args.append(more_arg)

    def add_args(self, more_args: List[str]):
        self.args.extend(more_args)

    def run(self) -> int:
        self.run_result = subprocess.run(self.args, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        return self.run_result.returncode

    def output(self) -> str:
        if self.run_result:
            return str(self.run_result.stdout)
        return ""

    def command_line(self) -> str:
        """Convenience method to return a string showing the command and arguments"""
        return " ".join(self.args)
        pass


class TempFileGuard:
    def __init__(self, base_name: str, extension: str):
        global _temp_dir
        if not _temp_dir or not _temp_dir():
            self.temp_dir = tempfile.TemporaryDirectory()
            _temp_dir = weakref.ref(self.temp_dir)
        else:
            self.temp_dir = _temp_dir()
        self.filename = os.path.join(self.temp_dir.name, base_name + extension)


class TempVDSGuard(TempFileGuard):
    def __init__(self, base_name="import_test"):
        super().__init__(base_name, ".vds")


class TempScanFileGuard(TempFileGuard):
    def __init__(self, base_name="scan_test"):
        super().__init__(base_name, ".scan.json")


@pytest.fixture
def hh_test_data_dir() -> str:
    return test_data_dir_hh


@pytest.fixture
def tt_test_data_dir() -> str:
    return test_data_dir_tt


@pytest.fixture
def segyimport_test_data_dir() -> str:
    return os.path.join(test_data_dir, "Plugins", "ImportPlugins", "SEGYUnittest")


@pytest.fixture
def platform_integration_test_data_dir() -> str:
    return os.path.join(test_data_dir_hh, "PlatformIntegration")


@pytest.fixture
def volve_data_dir() -> str:
    return "D:\\SEGY\\Equinor\\Volve"


def platform_integration_test_data_dir_non_fixture() -> str:
    """Need a version of this that's not a fixture because you can't use a fixture for test parameterization"""
    return os.path.join(test_data_dir_hh, "PlatformIntegration")
