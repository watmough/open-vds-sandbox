import openvds
import openvds_test
import pytest

import numpy as np

from CreateVds import *

class RequestData:
    def __init__(self):
        self.request = None
        self.minPos = (0,0,0)
        self.maxPos = (0,0,0)

def make_min_max(dimension_sizes, generator, abs_min, abs_max):
    min = [0,0,0]
    max = [0,0,0]
    for i in range(len(dimension_sizes)):
        width = generator.integers(abs_min[i], abs_max[i])
        pos = generator.integers(-width + 1, dimension_sizes[i] - 1)
        pos_max = pos + width
        if pos < 0:
            pos = 0
        if pos_max > dimension_sizes[i]:
            pos_max = dimension_sizes[i]
        min[i] = pos
        max[i] = pos_max
        assert pos_max > pos

    return (tuple(min), tuple(max))

def do_requests(acc, layout, requestsData):
    for requestData in requestsData:
        requestData.request = acc.requestVolumeSubset(requestData.minPos, requestData.maxPos)
    for requestData in requestsData:
        requestData.request.waitForCompletion()

def test_request_volume_subset_simple(benchmark, dataset_400_r32_64):
    with openvds.open(dataset_400_r32_64, "") as handle:
        acc = openvds.VolumeDataAccessManager(handle)
        layout = openvds.getLayout(handle)
        assert layout.getDimensionality() == 3
        dimension_sizes = np.array(layout.numSamples)
        generator = np.random.Generator(np.random.MT19937(dimension_sizes[0]))
        requestsData = []
        for i in range(16):
            requestsData.append(RequestData())
            requestData = requestsData[-1]
            (min, max) = make_min_max(dimension_sizes, generator, (1,1,1), dimension_sizes // 2)
            requestData.minPos = min
            requestData.maxPos = max
        benchmark(do_requests, acc, layout, requestsData)

