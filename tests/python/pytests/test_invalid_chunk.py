import openvds
import openvds_test
import pytest

from CreateVds import *

@pytest.mark.xfail(raises=openvds.ReadErrorException)
def test_read_empty_chunk(dataset_80_r32_64):
    with openvds_test.enableIoError() as errorHandle:
        errorHandle.setError("Dimensions_012LOD0/0", [], [1], 0, "")
        with openvds.open(dataset_80_r32_64,"") as handle:
            acc = openvds.VolumeDataAccessManager(handle)
            accessor = acc.createVolumeData3DReadAccessorR32(openvds.DimensionsND.Dimensions_012)
            v = accessor.getValue((1,1,1))

