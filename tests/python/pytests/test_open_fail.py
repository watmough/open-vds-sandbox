import openvds
import openvds_test
import pytest

from CreateVds import *

def test_open_fail_non_existing_vds():
    with pytest.raises(RuntimeError):
      with openvds.open("inmemory://doesnt_exist","") as handle:
          acc = openvds.VolumeDataAccessManager(handle)
          accessor = acc.createVolumeData3DReadAccessorR32(openvds.DimensionsND.Dimensions_012)
          v = accessor.getValue((1,1,1))

def test_open_fail_on_failed_to_parse_VolumeDataLayout(dataset_80_r32_64):
    with pytest.raises(RuntimeError):
      with openvds_test.enableIoError() as errorHandle:
          errorHandle.setError("VolumeDataLayout", [], [1,2,3,4,5,6], 0, "")
          with openvds.open(dataset_80_r32_64,"") as handle:
              acc = openvds.VolumeDataAccessManager(handle)
              accessor = acc.createVolumeData3DReadAccessorR32(openvds.DimensionsND.Dimensions_012)
              v = accessor.getValue((1,1,1))
