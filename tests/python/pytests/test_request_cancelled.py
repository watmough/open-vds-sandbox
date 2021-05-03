import openvds
import openvds_test
import pytest

from CreateVds import *

def test_request_cancelled(dataset_400_r32_64):
    with openvds.open(dataset_400_r32_64, "") as handle:
        acc = openvds.VolumeDataAccessManager(handle)
        r = acc.requestVolumeSubset((0,0,0),(400,400,400))


