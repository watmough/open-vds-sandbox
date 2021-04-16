import openvds
import openvds_test
import pytest

from CreateVds import *

@pytest.mark.xfail(raises=openvds.IndexOutOfRangeException)
def test_index_out_of_range_exception(dataset_80_r32_64):
    with openvds.open(dataset_80_r32_64,"") as handle:
        acc = openvds.VolumeDataAccessManager(handle)
        accessor = acc.createVolumeData3DReadAccessorR32(openvds.DimensionsND.Dimensions_012)
        v = accessor.getValue((100,100,100))

