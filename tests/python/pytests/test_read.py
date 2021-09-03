import openvds
import openvds_test
import pytest

from CreateVds import *

def test_read_accessor(dataset_80_r32_64):
    with openvds.open(dataset_80_r32_64, "") as handle:
        acc = openvds.VolumeDataAccessManager(handle)
        accessor = acc.createVolumeData3DReadAccessorR32(openvds.DimensionsND.Dimensions_012)
        v0 = accessor.getValue((0,0,0))
        assert v0 == 0.0
        v1 = accessor.getValue((0,0,1))
        assert v1 == 1.0
        v2 = accessor.getValue((0,0,2))
        assert v2 == 2.0
        v3 = accessor.getValue((0,0,3))
        assert v3 == 3.0

        v4 = accessor.getValue((0,1,0))
        assert v4 == 80.0
        v5 = accessor.getValue((0,2,0))
        assert v5 == 80.0 * 2.0
        v6 = accessor.getValue((0,3,0))
        assert v6 == 80.0 * 3.0

        v7 = accessor.getValue((1,0,0))
        assert v7 == 80.0 * 80.0 * 1.0
        v8 = accessor.getValue((2,0,0))
        assert v8 == 80.0 * 80.0 * 2.0
        v9 = accessor.getValue((3,0,0))
        assert v9 == 80.0 * 80.0 * 3.0


def test_read_traces(dataset_80_r32_64):
    with openvds.open(dataset_80_r32_64, "") as handle:
        acc = openvds.VolumeDataAccessManager(handle)
        r = acc.requestVolumeTraces([(0, 10.5, 10.5), (0, 20.5, 20.5)], 0)
        assert r.data[0] == 80 * 10 + 80 * 80 * 10
        assert r.data[80] == 80 * 20 + 80 * 80 * 20
