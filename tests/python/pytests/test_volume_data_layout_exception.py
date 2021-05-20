import openvds
import openvds_test
import pytest

from CreateVds import *

@pytest.mark.xfail(raises=openvds.InvalidArgument)
def test_getChannelValueRangeMinException(dataset_80_r32_64):
    with openvds.open(dataset_80_r32_64,"") as handle:
        layout = openvds.getLayout(handle)
        rangeMin = layout.getChannelValueRangeMin(4)

@pytest.mark.xfail(raises=openvds.InvalidArgument)
def test_getChannelName(dataset_80_r32_64):
    with openvds.open(dataset_80_r32_64,"") as handle:
        layout = openvds.getLayout(handle)
        name = layout.getChannelName(5)

