"""
Global configuration information for SEGYImport pytests
"""
import os
import subprocess
from typing import List

test_output_dir = "c:\\temp\\SEGY\\t"
test_data_dir = "c:\\temp\\SEGY\\RegressionTestData"


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