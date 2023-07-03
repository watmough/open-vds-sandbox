/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataChannelDescriptor.h>
#include <VDS/VolumeDataStore.h>
#include <VDS/DataBlock.h>
#include <VDS/VolumeDataLayer.h>
#include <IO/File.h>
#include <OpenVDS/ValueConversion.h>
#include <OpenVDS/Range.h>
#include <VDS/VolumeDataStore.h>

#include <cstdlib>
#include <cmath>

#include <gtest/gtest.h>


template<typename T>
float getErrorDelta(const OpenVDS::QuantizingValueConverterWithNoValue<T, float, false> &resultConverter, const void *referenceDataPtr, const void *waveletDataPtr, int offset)
{
  T waveletValue = *(reinterpret_cast<const T *>(waveletDataPtr) + offset);
  float waveletFloatValue = float(waveletValue);

  float referenceValue = *(reinterpret_cast<const float *>(referenceDataPtr) + offset);
  T referenceValueConverted = resultConverter.ConvertValue(referenceValue);
  float referenceValueConvertedFloat = float(referenceValueConverted);
  return waveletFloatValue - referenceValueConvertedFloat;
}

static void Stats(const OpenVDS::FloatRange &valueRange, const OpenVDS::DataBlock &referenceDataBlock, const std::vector<uint8_t> &referenceData, const OpenVDS::DataBlock &waveletDataBlock, const std::vector<uint8_t> &waveletData, float &diff, float &maxError, float &deviation, float &samples)
{
  diff = 0.0;
  samples = 0.0;

  assert(referenceDataBlock.Size[0] == waveletDataBlock.Size[0] &&
         referenceDataBlock.Size[1] == waveletDataBlock.Size[1] &&
         referenceDataBlock.Size[2] == waveletDataBlock.Size[2]);

  assert(referenceDataBlock.Format == OpenVDS::VolumeDataFormat::Format_R32);

  OpenVDS::VolumeDataChannelDescriptor::Format waveletFormat = waveletDataBlock.Format;
  const void *referenceDataPtr = referenceData.data();
  
  const void *waveletDataPtr = waveletData.data();

  float localAverage = 0.0;
  maxError = 0.0;

  OpenVDS::QuantizingValueConverterWithNoValue<uint8_t, float, false> resultConverter8(valueRange.Min, valueRange.Max, 1.0f, 0.0f, 0.0f, 0.0f);
  OpenVDS::QuantizingValueConverterWithNoValue<uint16_t, float, false> resultConverter16(valueRange.Min, valueRange.Max, 1.0f, 0.0f, 0.0f, 0.0f);

  for (int i2 = 0; i2 < referenceDataBlock.Size[2]; i2++)
  {
    for (int i1 = 0; i1 < referenceDataBlock.Size[1]; i1++)
    {
      for (int i0 = 0; i0 < referenceDataBlock.Size[0]; i0++)
      {
        int32_t offset = i0 + i1 * referenceDataBlock.Pitch[1] + i2 * referenceDataBlock.Pitch[2];

        float errorDelta;

        if (waveletFormat == OpenVDS::VolumeDataFormat::Format_R32)
        {
          errorDelta = *((float *)waveletDataPtr + offset) - *((float *)referenceDataPtr + offset);
        }
        else if (waveletFormat == OpenVDS::VolumeDataFormat::Format_U16)
        {
          errorDelta = getErrorDelta<uint16_t>(resultConverter16, referenceDataPtr, waveletDataPtr, offset);
        }
        else if (waveletFormat == OpenVDS::VolumeDataFormat::Format_U8)
        {
          errorDelta = getErrorDelta<uint8_t>(resultConverter8, referenceDataPtr, waveletDataPtr, offset);
        }
        else
        {
          fprintf(stderr, "Illegal datablock format!");
          abort();
        }

        if (fabs(errorDelta) > maxError) maxError = fabs(errorDelta);

        diff += fabs(errorDelta);
        localAverage += fabs(errorDelta);
        samples+=1.0;
      }
    }
  }

  float average = localAverage / samples;
  deviation = 0.0;

  for (int i2 = 0; i2 < referenceDataBlock.Size[2]; i2++)
  { for (int i1 = 0; i1 < referenceDataBlock.Size[1]; i1++)
    {
      for (int i0 = 0; i0 < referenceDataBlock.Size[0]; i0++)
      {
        int32_t offset = i0 + i1 * referenceDataBlock.Pitch[1] + i2 * referenceDataBlock.Pitch[2];

        float errorDelta;

        if (waveletFormat == OpenVDS::VolumeDataFormat::Format_R32)
        {
          errorDelta = *(((float *)waveletDataPtr) + offset) - *(((float *)referenceDataPtr) + offset);
        }
        else if (waveletFormat == OpenVDS::VolumeDataFormat::Format_U16)
        {
          errorDelta = getErrorDelta<uint16_t>(resultConverter16, referenceDataPtr, waveletDataPtr, offset);
        }
        else if (waveletFormat == OpenVDS::VolumeDataFormat::Format_U8)
        {
          errorDelta = getErrorDelta<uint8_t>(resultConverter8, referenceDataPtr, waveletDataPtr, offset);
        }
        else
        {
          fprintf(stderr, "Illegal datablock format!");
          abort();
        }
        deviation += (average - fabs(errorDelta)) * (average - fabs(errorDelta));
      }
    }
  }
}

static std::vector<uint8_t> LoadTestFile(const std::string &file)
{
    OpenVDS::File chunkFile;
    OpenVDS::Error error;
    chunkFile.Open(TEST_DATA_PATH + file, false, false, false, error);
    EXPECT_EQ(error.code, 0);

    int64_t fileSize = chunkFile.Size(error);
    EXPECT_EQ(error.code, 0);

    std::vector<uint8_t> serializedData;
    serializedData.resize(fileSize);
    chunkFile.Read(&serializedData[0], 0, (int32_t)fileSize, error);
    EXPECT_EQ(error.code, 0);
    return serializedData;
}

float getOnePercentValueRange(const OpenVDS::FloatRange &range, OpenVDS::VolumeDataChannelDescriptor::Format format)
{
  if (format == OpenVDS::VolumeDataFormat::Format_R32)
  {
    return (range.Max - range.Min) / 100.0f;
  }
  else if (format == OpenVDS::VolumeDataFormat::Format_U8)
  {
    return float(std::numeric_limits<uint8_t>::max()) / 100.0f;
  }
  else if (format == OpenVDS::VolumeDataFormat::Format_U16)
  {
    return float(std::numeric_limits<uint16_t>::max()) / 100.0f;
  }

  return 0.0;
}

void verify_wavelet(const OpenVDS::FloatRange &valueRange, const OpenVDS::DataBlock &dataBlockNone, const std::vector<uint8_t> &dataNone, const std::string &file, OpenVDS::VolumeDataChannelDescriptor::Format sourceFormat, OpenVDS::VolumeDataChannelDescriptor::Format loadFormat)
{
  OpenVDS::Error error;
  std::vector<uint8_t> serializedWavelet = LoadTestFile(file);
  std::vector<uint8_t> dataWavelet;
  OpenVDS::DataBlock dataBlockWavelet;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedWavelet, sourceFormat, loadFormat, OpenVDS::CompressionMethod::Wavelet, valueRange, 1.0f, 0.0f, false, 0.0f, 0.0f, 0, dataBlockWavelet, dataWavelet, error);
  EXPECT_EQ(error.code, 0);

  int dataNoneScale = OpenVDS::GetVoxelFormatByteSize(dataBlockNone.Format);
  int compressedDataScale = OpenVDS::GetVoxelFormatByteSize(loadFormat);
  EXPECT_EQ(dataNone.size() / dataNoneScale, dataWavelet.size() / compressedDataScale);

  float diff;
  float maxError;
  float deviation;
  float samples = 0;
  Stats(valueRange, dataBlockNone, dataNone, dataBlockWavelet, dataWavelet, diff, maxError, deviation, samples);
  //double variance = deviation / samples;
  //double std_dev = sqrt(variance);
  double avg_diff = diff / samples;
  double one_procent_range = getOnePercentValueRange(valueRange, loadFormat);
  EXPECT_TRUE(avg_diff < one_procent_range * 2);
}

template<typename T>
void verify_lossless_typed(const OpenVDS::FloatRange &valueRange, const OpenVDS::DataBlock &referenceDataBlock, const std::vector<uint8_t> &referenceData, const std::vector<uint8_t> &deserializedData)
{
  OpenVDS::QuantizingValueConverterWithNoValue<T, float, false> resultConverter(valueRange.Min, valueRange.Max, 1.0f, 0.0f, 0.0f, 0.0f);
  const T* deserializedDataPtr = reinterpret_cast<const T*>(deserializedData.data());
  const float* referenceDataPtr = reinterpret_cast<const float*>(referenceData.data());
  for (int i2 = 0; i2 < referenceDataBlock.Size[2]; i2++)
  {
    for (int i1 = 0; i1 < referenceDataBlock.Size[1]; i1++)
    {
      for (int i0 = 0; i0 < referenceDataBlock.Size[0]; i0++)
      {
        int32_t offset = i0 + i1 * referenceDataBlock.Pitch[1] + i2 * referenceDataBlock.Pitch[2];
        T convertedReferenceValue = resultConverter.ConvertValue(referenceDataPtr[offset]);
        T deserializedValue = deserializedDataPtr[offset];
        if (deserializedValue != convertedReferenceValue)
        {
          fmt::print(stderr, "Failed to verify equality: non_compression = {}, compressed = {}\n", convertedReferenceValue, deserializedValue);
          EXPECT_TRUE(false);
        }
      }
    }
  }
}

void verify_lossless(const OpenVDS::FloatRange &valueRange, const OpenVDS::DataBlock &referenceDataBlock, const std::vector<uint8_t> &referenceData, const std::string &file, OpenVDS::CompressionMethod compressionMethod, OpenVDS::VolumeDataChannelDescriptor::Format sourceFormat, OpenVDS::VolumeDataChannelDescriptor::Format loadFormat)
{
  OpenVDS::Error error;
  std::vector<uint8_t> serializedLossless = LoadTestFile(file);
  std::vector<uint8_t> deserializedData;
  OpenVDS::DataBlock deserializedDataBlock;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedLossless, sourceFormat, loadFormat, compressionMethod, valueRange, 1.0f, 0.0f, false, 0.0f, 0.0f, -1, deserializedDataBlock, deserializedData, error);
  EXPECT_EQ(error.code, 0);

  int referenceTypeScale = OpenVDS::GetVoxelFormatByteSize(referenceDataBlock.Format);
  int deserializedTypeScale = OpenVDS::GetVoxelFormatByteSize(deserializedDataBlock.Format);
  EXPECT_EQ(referenceData.size() / referenceTypeScale, deserializedData.size() / deserializedTypeScale);
  if (referenceDataBlock.Format == loadFormat)
  {
    int comp_lossless = memcmp(referenceData.data(), deserializedData.data(), referenceData.size());
    EXPECT_TRUE(comp_lossless == 0);
  }
  else
  {
    if (loadFormat == OpenVDS::VolumeDataFormat::Format_U8)
    {
      verify_lossless_typed<uint8_t>(valueRange, referenceDataBlock, referenceData, deserializedData);
    }
    else if (loadFormat == OpenVDS::VolumeDataFormat::Format_U16)
    {
      verify_lossless_typed<uint16_t>(valueRange, referenceDataBlock, referenceData, deserializedData);
    }
    else
    {
      fmt::print(stderr, "Unsuported compare of data in different formats.\n");
      EXPECT_TRUE(false);
    }
  }
}

GTEST_TEST(VDS_integration, DeSerializeVolumeData)
{
  OpenVDS::Error error;

  OpenVDS::VolumeDataFormat sourceFormat = OpenVDS::VolumeDataFormat::Format_R32;
  OpenVDS::FloatRange valueRange(-0.07883811742067337f, 0.07883811742067337f);
  std::vector<uint8_t> serializedNone = LoadTestFile("/chunk.CompressionMethod_None");
  std::vector<uint8_t> dataNone;
  OpenVDS::DataBlock dataBlockNone;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedNone, sourceFormat,sourceFormat, OpenVDS::CompressionMethod::None, valueRange, 1.0f, 0.0f, false, 0.0f, 0.0f, 0, dataBlockNone, dataNone, error);
  EXPECT_EQ(error.code, 0);
  
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.CompressionMethod_RLE", OpenVDS::CompressionMethod::RLE, sourceFormat, sourceFormat);
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.CompressionMethod_Zip", OpenVDS::CompressionMethod::Zip, sourceFormat, sourceFormat);
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.CompressionMethod_WaveletLossless", OpenVDS::CompressionMethod::WaveletLossless, sourceFormat, sourceFormat);
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.CompressionMethod_WaveletLossless_1_6", OpenVDS::CompressionMethod::WaveletLossless, sourceFormat, sourceFormat);
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.U8.CompressionMethod_None", OpenVDS::CompressionMethod::None, sourceFormat, OpenVDS::VolumeDataFormat::Format_U8);
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.U16.CompressionMethod_None", OpenVDS::CompressionMethod::None, sourceFormat, OpenVDS::VolumeDataFormat::Format_U16);
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.U8.CompressionMethod_WaveletLossless_1_6", OpenVDS::CompressionMethod::WaveletLossless, sourceFormat, OpenVDS::VolumeDataFormat::Format_U8);
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.U16.CompressionMethod_WaveletLossless_1_6",OpenVDS::CompressionMethod::WaveletLossless, sourceFormat, OpenVDS::VolumeDataFormat::Format_U16);

  verify_wavelet(valueRange, dataBlockNone, dataNone, "/chunk.CompressionMethod_Wavelet_1_6",  sourceFormat, sourceFormat);
  verify_wavelet(valueRange, dataBlockNone, dataNone, "/chunk.U8.CompressionMethod_Wavelet_1_6", sourceFormat, OpenVDS::VolumeDataFormat::Format_U8);
  verify_wavelet(valueRange, dataBlockNone, dataNone, "/chunk.U16.CompressionMethod_Wavelet_1_6", sourceFormat, OpenVDS::VolumeDataFormat::Format_U16);

}

GTEST_TEST(VDS_integration, DeSerializeVolumeData1082)
{
  OpenVDS::Error error;

  OpenVDS::FloatRange valueRange(-0.07883811742067337f, 0.07883811742067337f);
  std::vector<uint8_t> serializedNone = LoadTestFile("/chunk.1082.CompressionMethod_None");
  std::vector<uint8_t> dataNone;
  OpenVDS::DataBlock dataBlockNone;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedNone, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::None, valueRange, 1.0f, 0.0f, false, 0.0f, 0.0f, 0, dataBlockNone, dataNone, error);
  EXPECT_EQ(error.code, 0);
  
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.1082.CompressionMethod_WaveletLossless", OpenVDS::CompressionMethod::WaveletLossless, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32);
}

GTEST_TEST(VDS_integration, DeSerializeVolumeData1255)
{
  OpenVDS::Error error;

  OpenVDS::FloatRange valueRange(-0.07883811742067337f, 0.07883811742067337f);
  std::vector<uint8_t> serializedNone = LoadTestFile("/chunk.1255.CompressionMethod_None");
  std::vector<uint8_t> dataNone;
  OpenVDS::DataBlock dataBlockNone;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedNone, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::None, valueRange, 1.0f, 0.0f, false, 0.0f, 0.0f, 0, dataBlockNone, dataNone, error);
  EXPECT_EQ(error.code, 0);
  
  verify_lossless(valueRange, dataBlockNone, dataNone, "/chunk.1255.CompressionMethod_WaveletLossless", OpenVDS::CompressionMethod::WaveletLossless, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32);
}

GTEST_TEST(VDS_integration, DeSerializeVolumeDataNoValue)
{
  OpenVDS::Error error;

  OpenVDS::FloatRange valueRange(-0.1234f, 0.1234f);
  std::vector<uint8_t> serializedNone = LoadTestFile("/chunk.Dimensions_012LOD0_19_CompressionMethod_None_no_value");
  std::vector<uint8_t> dataNone;
  OpenVDS::DataBlock dataBlockNone;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedNone, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::None, valueRange, 1.0f, 0.0f, true, 44.50f, 44.50f, 0,dataBlockNone, dataNone, error);
  EXPECT_EQ(error.code, 0);
  
  std::vector<uint8_t> serializedLossless = LoadTestFile("/chunk.Dimensions_012LOD0_19_CompressionMethod_WaveletLossless_no_value");
  std::vector<uint8_t> deserializedData;
  OpenVDS::DataBlock deserializedDataBlock;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedLossless, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::WaveletLossless, valueRange, 1.0f, 0.0f, true, 44.50f, 44.50f, -1, deserializedDataBlock, deserializedData, error);
  EXPECT_EQ(error.code, 0);

  int comp_lossless = memcmp(dataNone.data(), deserializedData.data(), dataNone.size());
  EXPECT_TRUE(comp_lossless == 0);
}

GTEST_TEST(VDS_integration, DeSerializeWaveletToUint8_16)
{
  OpenVDS::Error error;

  OpenVDS::FloatRange valueRange(-0.07883811742067337f, 0.07883811742067337f);
  std::vector<uint8_t> serializedNone = LoadTestFile("/chunk.CompressionMethod_None");
  std::vector<uint8_t> dataNone;
  OpenVDS::DataBlock dataBlockNone;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedNone, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::None, valueRange, 1.0f, 0.0f, false, 0.0f, 0.0f, 0, dataBlockNone, dataNone, error);
  EXPECT_EQ(error.code, 0);
  
  verify_wavelet(valueRange, dataBlockNone, dataNone, "/chunk.CompressionMethod_WaveletLossless_1_6", OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_U8);
}

GTEST_TEST(VDS_integration, DeSerializeVolumeDataReplacementNoValue)
{
  OpenVDS::Error error;

  OpenVDS::FloatRange valueRange(-0.1234f, 0.1234f);
  std::vector<uint8_t> originalSerializedNone = LoadTestFile("/chunk.Dimensions_012LOD0_19_CompressionMethod_None_no_value");
  std::vector<uint8_t> originalDataNone;
  OpenVDS::DataBlock originalDataBlockNone;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(originalSerializedNone, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::None, valueRange, 1.0f, 0.0f, true, 44.50f, 44.50f, 0, originalDataBlockNone, originalDataNone, error);
  EXPECT_EQ(error.code, 0);

  std::vector<uint8_t> serializedNone = LoadTestFile("/chunk.Dimensions_012LOD0_19_CompressionMethod_None_no_value");
  std::vector<uint8_t> dataNone;
  OpenVDS::DataBlock dataBlockNone;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedNone, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::None, valueRange, 1.0f, 0.0f, true, 44.50f, 22.25f, 0,dataBlockNone, dataNone, error);
  EXPECT_EQ(error.code, 0);
  
  std::vector<uint8_t> serializedLossless = LoadTestFile("/chunk.Dimensions_012LOD0_19_CompressionMethod_WaveletLossless_no_value");
  std::vector<uint8_t> deserializedData;
  OpenVDS::DataBlock deserializedDataBlock;
  OpenVDS::VolumeDataStore::DeserializeVolumeData(serializedLossless, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::CompressionMethod::WaveletLossless, valueRange, 1.0f, 0.0f, true, 44.50f, 22.25f, -1, deserializedDataBlock, deserializedData, error);
  EXPECT_EQ(error.code, 0);

  int comp_lossless = memcmp(dataNone.data(), deserializedData.data(), dataNone.size());
  EXPECT_TRUE(comp_lossless == 0);

  int comparedNoValues = 0;
  float* originalFloatBuffer = (float*)originalDataNone.data();
  float* noneFloatBuffer = (float*)dataNone.data();
  float* deserializedFloatBuffer = (float*)deserializedData.data();
  for (int i2 = 0; i2 < originalDataBlockNone.Size[2]; i2++)
  {
    for (int i1 = 0; i1 < originalDataBlockNone.Size[1]; i1++)
    {
      for (int i0 = 0; i0 < originalDataBlockNone.Size[0]; i0++)
      {
        int32_t offset = i0 + i1 * originalDataBlockNone.Pitch[1] + i2 * originalDataBlockNone.Pitch[2];
        if (originalFloatBuffer[offset] == 44.50f)
        {
          comparedNoValues++;
          EXPECT_EQ(deserializedFloatBuffer[offset], 22.25f);
          EXPECT_EQ(noneFloatBuffer[offset], 22.25f);
        }
      }
    }
  }
  EXPECT_GT(comparedNoValues, 0);
}
