#include <benchmark/benchmark.h>
#include <OpenVDS/OpenVDS.h>
#include "../utils/GenerateVDS.h"

#include <stdexcept>

static int datasetSize = 120;
static int requestCount = 16;

void setupNoiseTestHandle(OpenVDS::ScopedVDSHandle& handle)
{
  OpenVDS::Error error;
  handle = generateSimpleInMemory3DVDS(datasetSize, datasetSize, datasetSize);

  fill3DVDSWithNoise(handle);
}
struct RequestData
{
  std::shared_ptr<OpenVDS::VolumeDataRequest> request;
  int32_t minPos[OpenVDS::Dimensionality_Max];
  int32_t maxPos[OpenVDS::Dimensionality_Max];
  int32_t voxelCount;
  std::vector<float> bufferFloat;
};
static void benchmark_request_volume_subset(benchmark::State& state) {
  OpenVDS::ScopedVDSHandle handle;
  setupNoiseTestHandle(handle);
  std::vector<RequestData> requestsData(requestCount);
  std::mt19937 gen(datasetSize);
  std::uniform_int_distribution<> width_dist(datasetSize / 4, datasetSize / 2);
  std::uniform_int_distribution<> offset_dist(0, datasetSize / 2);

  for (auto& requestData : requestsData)
  {
    requestData.minPos[0] = offset_dist(gen);
    requestData.minPos[1] = offset_dist(gen);
    requestData.minPos[2] = offset_dist(gen);
    requestData.maxPos[0] = requestData.minPos[0] + width_dist(gen);
    requestData.maxPos[1] = requestData.minPos[1] + width_dist(gen);
    requestData.maxPos[2] = requestData.minPos[2] + width_dist(gen);
    requestData.voxelCount = (requestData.maxPos[0] - requestData.minPos[0]) * (requestData.maxPos[1] - requestData.minPos[1]) * (requestData.maxPos[2] - requestData.minPos[2]);
    requestData.bufferFloat.resize(requestData.voxelCount);
  }

  auto accessManager = OpenVDS::GetAccessManager(handle);
  for (auto _ : state)
  {
    for (auto& requestData : requestsData)
    {
      requestData.request = accessManager.RequestVolumeSubset<float>(requestData.bufferFloat.data(), requestData.bufferFloat.size() * sizeof(float), OpenVDS::Dimensions_012, 0, 0, requestData.minPos, requestData.maxPos);
    }
    for (auto& requestData : requestsData)
    {
      if (!requestData.request->WaitForCompletion())
        throw std::runtime_error("Requests failed!!");
    }
  }
  state.counters["request_count"] = requestCount;
}
  

BENCHMARK(benchmark_request_volume_subset);

