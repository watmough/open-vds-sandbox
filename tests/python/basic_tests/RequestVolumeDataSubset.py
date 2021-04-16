import openvds
import numpy as np
from connection_defs import *
import openvds_test
import pytest

from CreateVds import *

CreateVds("small_set")

handle = openvds.open("inmemory://small_set", "")
acc = openvds.VolumeDataAccessManager(handle)

r = acc.requestVolumeSubset((0,0,0),(100,100,100))

accessor = acc.createVolumeData3DReadAccessorR32(openvds.DimensionsND.Dimensions_012)
try:
    v = accessor.getValue((100,100,100))
except  openvds.IndexOutOfRangeException as e:
    print("Caught e")

openvds.close(handle)
