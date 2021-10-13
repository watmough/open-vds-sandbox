"""
Global configuration information for SEGYImport pytests
"""
import os
import subprocess
import tempfile
import weakref
import pytest
from typing import List

test_data_dir = "c:\\temp\\SEGY\\RegressionTestData"

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
def teleport_test_data_dir() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "Teleport")


@pytest.fixture
def segyimport_test_data_dir() -> str:
    return os.path.join(test_data_dir, "Plugins", "ImportPlugins", "SEGYUnittest")


def platform_integration_test_data_dir() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration")
