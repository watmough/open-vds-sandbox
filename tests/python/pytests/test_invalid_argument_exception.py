import openvds
import pytest

from CreateVds import *

def test_invalid_argument(dataset_80_r32_64):
    with pytest.raises(openvds.InvalidArgument):
      with openvds.open(dataset_80_r32_64,"") as handle:
          acc = openvds.VolumeDataAccessManager(handle)
          r = acc.requestVolumeSubset((0,0,0),(100,70,70))
          r.waitForCompletion()

