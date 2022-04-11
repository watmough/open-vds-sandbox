import openvds
import openvds_test
import pytest

from CreateVds import *

def test_getChannelValueRangeMinException(dataset_80_r32_64):
    with pytest.raises(openvds.InvalidArgument):
      with openvds.open(dataset_80_r32_64,"") as handle:
          layout = openvds.getLayout(handle)
          rangeMin = layout.getChannelValueRangeMin(4)

def test_getChannelName(dataset_80_r32_64):
    with pytest.raises(openvds.InvalidArgument):
      with openvds.open(dataset_80_r32_64,"") as handle:
          layout = openvds.getLayout(handle)
          name = layout.getChannelName(5)

