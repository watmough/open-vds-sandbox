import openvds
import openvds_test
import pytest

from CreateVds import *

@pytest.mark.xfail(raises=openvds.InvalidArgument)
def test_invalid_argument(dataset_80_r32_64):
    with openvds.open(dataset_80_r32_64,"") as handle:
        acc = openvds.VolumeDataAccessManager(handle)
        r = acc.requestVolumeSubset((0,0,0),(100,70,70))
        r.waitForCompletion()

