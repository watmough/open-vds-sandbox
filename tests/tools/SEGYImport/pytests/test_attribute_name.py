import os
import tempfile
from pathlib import Path

import pytest
import openvds

from segyimport_test_config import test_data_dir, ImportExecutor


class TempVDSGuard:
    def __init__(self, base_name="import_test"):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.filename = os.path.join(self.temp_dir.name, base_name + ".vds")


@pytest.fixture
def poststack_segy() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "Teleport", "Teleport_Trim",
                        "3D_Stack", "ST0202R08_TIME.segy")


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard()


def test_default_attribute_and_unit(poststack_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)
        descriptor = layout.getChannelDescriptor(0)
        assert descriptor.name == "Amplitude"
        assert descriptor.unit == ""


def test_custom_attribute_name(poststack_segy, output_vds):
    custom_attribute_name = "Vrms"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds.filename])
    ex.add_args(["--attribute-name", custom_attribute_name])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)
        descriptor = layout.getChannelDescriptor(0)
        assert descriptor.name == custom_attribute_name
        assert descriptor.unit == ""


def test_custom_attribute_unit(poststack_segy, output_vds):
    custom_attribute_unit = "Hz"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds.filename])
    ex.add_args(["--attribute-unit", custom_attribute_unit])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)
        descriptor = layout.getChannelDescriptor(0)
        assert descriptor.name == "Amplitude"
        assert descriptor.unit == custom_attribute_unit


def test_invalid_attribute_name(poststack_segy, output_vds):
    custom_attribute_name = "JulesVerne"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds.filename])
    ex.add_args(["--attribute-name", custom_attribute_name])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result > 0, ex.output()
    assert not Path(output_vds.filename).exists()


def test_invalid_attribute_unit(poststack_segy, output_vds):
    custom_attribute_unit = "furlong/fortnight"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds.filename])
    ex.add_args(["--attribute-unit", custom_attribute_unit])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result > 0, ex.output()
    assert not Path(output_vds.filename).exists()
