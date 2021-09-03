import os
from pathlib import Path

import pytest
import openvds

from segyimport_test_config import test_data_dir, test_output_dir, ImportExecutor


@pytest.fixture
def poststack_segy() -> str:
    return os.path.join(test_data_dir, "HeadwavePlatform", "PlatformIntegration", "Teleport", "Teleport_Trim",
                        "3D_Stack", "ST0202R08_TIME.segy")


@pytest.fixture
def output_vds() -> str:
    vds_filename = os.path.join(test_output_dir, "import_test.vds")
    if Path(vds_filename).exists():
        os.remove(vds_filename)
    return vds_filename


def test_default_attribute_and_unit(poststack_segy, output_vds):
    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds).exists()

    with openvds.open(output_vds, "") as handle:
        layout = openvds.getLayout(handle)
        descriptor = layout.getChannelDescriptor(0)
        assert descriptor.name == "Amplitude"
        assert descriptor.unit == ""


def test_custom_attribute_name(poststack_segy, output_vds):
    custom_attribute_name = "Vrms"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds])
    ex.add_args(["--attribute-name", custom_attribute_name])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds).exists()

    with openvds.open(output_vds, "") as handle:
        layout = openvds.getLayout(handle)
        descriptor = layout.getChannelDescriptor(0)
        assert descriptor.name == custom_attribute_name
        assert descriptor.unit == ""


def test_custom_attribute_unit(poststack_segy, output_vds):
    custom_attribute_unit = "Hz"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds])
    ex.add_args(["--attribute-unit", custom_attribute_unit])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds).exists()

    with openvds.open(output_vds, "") as handle:
        layout = openvds.getLayout(handle)
        descriptor = layout.getChannelDescriptor(0)
        assert descriptor.name == "Amplitude"
        assert descriptor.unit == custom_attribute_unit


def test_invalid_attribute_name(poststack_segy, output_vds):
    custom_attribute_name = "JulesVerne"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds])
    ex.add_args(["--attribute-name", custom_attribute_name])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result > 0, ex.output()
    assert not Path(output_vds).exists()


def test_invalid_attribute_unit(poststack_segy, output_vds):
    custom_attribute_unit = "furlong/fortnight"

    ex = ImportExecutor()

    ex.add_args(["--vdsfile", output_vds])
    ex.add_args(["--attribute-unit", custom_attribute_unit])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result > 0, ex.output()
    assert not Path(output_vds).exists()
