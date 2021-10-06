import os
from pathlib import Path
from typing import Tuple, Union

import pytest
import openvds

from segyimport_test_config import test_data_dir, ImportExecutor, TempVDSGuard


def construct_respace_executor(output_vds: TempVDSGuard, segy_name: str, respace_setting: Union[str, None]) -> ImportExecutor:
    ex = ImportExecutor()

    if respace_setting:
        ex.add_args(["--respace-gathers", respace_setting])

    ex.add_arg("--prestack")
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(segy_name)

    return ex


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_mutes_test")


@pytest.fixture
def prestack_segy() -> str:
    return os.path.join(test_data_dir, "Plugins", "ImportPlugins", "SEGYUnittest", "Mutes",
                        "")


@pytest.fixture
def default_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with no arg for respacing"""
    ex = construct_respace_executor(output_vds, prestack_segy, None)
    return ex, output_vds


@pytest.fixture
def auto_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with respacing set to Auto"""
    ex = construct_respace_executor(output_vds, prestack_segy, "Auto")
    return ex, output_vds


@pytest.fixture
def off_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with respacing set to Off"""
    ex = construct_respace_executor(output_vds, prestack_segy, "Off")
    return ex, output_vds


@pytest.fixture
def on_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with respacing set to On"""
    ex = construct_respace_executor(output_vds, prestack_segy, "On")
    return ex, output_vds


@pytest.fixture
def invalid_executor(prestack_segy, output_vds) -> Tuple[ImportExecutor, TempVDSGuard]:
    """Setup an ImportExecutor with respacing set to an invalid value"""
    ex = construct_respace_executor(output_vds, prestack_segy, "Partial")
    return ex, output_vds


def test_gather_spacing_invalid_arg(invalid_executor):
    ex, output_vds = invalid_executor

    result = ex.run()

    assert result != 0, ex.output()
    # TODO what text to look for?
    assert "some message" in ex.output().lower()


def test_gather_spacing_default(default_executor):
    ex, output_vds = default_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        # TODO read an inline (or all inlines)
        # TODO for each gather, check location of dead traces
        # TODO assert dead traces occur elsewhere besides the end of the gather

    assert False, "not implemented"


def test_gather_spacing_auto(auto_executor):
    ex, output_vds = auto_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        # TODO read an inline (or all inlines)
        # TODO for each gather, check location of dead traces
        # TODO assert dead traces occur elsewhere besides the end of the gather

    assert False, "not implemented"


def test_gather_spacing_on(on_executor):
    ex, output_vds = on_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        # TODO read an inline (or all inlines)
        # TODO for each gather, check location of dead traces
        # TODO assert dead traces occur elsewhere besides the end of the gather

    assert False, "not implemented"


def test_gather_spacing_off(off_executor):
    ex, output_vds = off_executor

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        # TODO read an inline (or all inlines)
        # TODO for each gather, check location of dead traces
        # TODO assert dead traces occur only at the end of the gather

    assert False, "not implemented"
