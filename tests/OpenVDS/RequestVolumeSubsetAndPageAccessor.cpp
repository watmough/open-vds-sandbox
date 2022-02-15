#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataAccess.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeData.h>

#include <array>

#include <gtest/gtest.h>

TEST(OpenVDS, MixedRequests)
{
  std::string url = TEST_URL;
  std::string connectionString = TEST_CONNECTION;
  if(url.empty())
  {
    GTEST_SKIP() << "Test Environment for connecting to VDS is not set";
  }

  OpenVDS::Error error;

  for (int i = 0; i < 20; i++)
  {
    OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(url, connectionString, error));
    ASSERT_TRUE(handle);

    OpenVDS::VolumeDataLayout* layout = OpenVDS::GetLayout(handle);

    OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

    std::vector<uint8_t> buffer;
    buffer.resize(accessManager.GetVolumeSubsetBufferSize({ 0, 0, 0 }, { 100, 100, 100 }, layout->GetChannelFormat(0)));
    auto request = accessManager.RequestVolumeSubset((void *)buffer.data(), buffer.size(), OpenVDS::Dimensions_012, 0, 0, { 0, 0, 0 }, { 100, 100, 100 }, layout->GetChannelFormat(0));

    auto dataAccessor = accessManager.CreateVolumeData3DReadAccessorR32(OpenVDS::Dimensions_012, 0, 0, 256, -1000);

    float value = dataAccessor.GetValue({ int(layout->GetDimensionNumSamples(2) / 1.44),int(layout->GetDimensionNumSamples(0) / 2.22), int(layout->GetDimensionNumSamples(1) / 1.55) });
    (void)value;

    request->WaitForCompletion();
  }
}
