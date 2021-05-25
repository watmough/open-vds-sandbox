#include <fmt/printf.h>

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataLayout.h>

#include <cxxopts/cxxopts.hpp>
#include <ThreadPool/ThreadPool.h>
#include <PrintHelpers.h>

#include <HelpConnection.h>

#include <assert.h>

inline char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  return in;
}

inline void printPercentage(float percentage)
{
  fmt::print(stdout, "\r {0:5.2f} % Done.", percentage);
  fflush(stdout);
}

inline int32_t GetVoxelFormatByteSize(OpenVDS::VolumeDataChannelDescriptor::Format format)
{
  int32_t iRetval = -1;
  switch (format) {
  case OpenVDS::VolumeDataChannelDescriptor::Format_R64:
  case OpenVDS::VolumeDataChannelDescriptor::Format_U64:
    iRetval = 8;
    break;
  case OpenVDS::VolumeDataChannelDescriptor::Format_R32:
  case OpenVDS::VolumeDataChannelDescriptor::Format_U32:
    iRetval = 4;
    break;
  case OpenVDS::VolumeDataChannelDescriptor::Format_U16:
    iRetval = 2;
    break;
  case OpenVDS::VolumeDataChannelDescriptor::Format_U8:
  case OpenVDS::VolumeDataChannelDescriptor::Format_1Bit:
    iRetval =1;
    break;
  default:
    fprintf(stderr, "Unknown voxel format");
    abort();
  }
  return iRetval;
}

struct CopyError
{
  int code = 0;
  std::string message;
};

bool flushFutureBufer(std::vector<std::future<CopyError>>& futures, bool jsonOutput, int64_t totalChunks, int64_t &doneChunks, int &percentage, CopyError& copyError)
{
  for (auto& future : futures)
  {
    auto error = future.get();
    if (error.code)
    {
      copyError = error;
      return false;
    }
    int newPercentage = int(++doneChunks * 10000 / totalChunks);
    if (newPercentage != percentage)
    {
      percentage = newPercentage;
      if (!jsonOutput)
      {
        float output_percentage = float(percentage) / 100;
        printPercentage(output_percentage);
      }
    }
  }
  futures.clear();
  return true;
}

int main(int argc, char **argv)
{
  cxxopts::Options options("VDSCopy", "VDSCopy - A tool for copying a VDS between locations\n\nUse -H or see online documentation for connection string paramters:\nhttp://osdu.pages.community.opengroup.org/platform/domain-data-mgmt-services/seismic/open-vds/connection.html\n");
  options.positional_help("<source_url> <destination_url>");

  std::vector<std::string> urlarg;
  std::string sourceConnection;
  std::string destinationConnection;
  std::string compressionMethodString;
  float compressionTolerance = std::nanf("nan");

  bool jsonOutput = false;
  bool help = false;
  bool helpConnection = false;
  bool version = false;

  std::string supportedCompressionMethods = "None";
  if(OpenVDS::IsCompressionMethodSupported(OpenVDS::CompressionMethod::Wavelet)) supportedCompressionMethods += ", Wavelet";
  if(OpenVDS::IsCompressionMethodSupported(OpenVDS::CompressionMethod::RLE)) supportedCompressionMethods += ", RLE";
  if(OpenVDS::IsCompressionMethodSupported(OpenVDS::CompressionMethod::Zip)) supportedCompressionMethods += ", Zip";
  if(OpenVDS::IsCompressionMethodSupported(OpenVDS::CompressionMethod::WaveletNormalizeBlock)) supportedCompressionMethods += ", WaveletNormalizeBlock";
  if(OpenVDS::IsCompressionMethodSupported(OpenVDS::CompressionMethod::WaveletLossless)) supportedCompressionMethods += ", WaveletLossless";
  if(OpenVDS::IsCompressionMethodSupported(OpenVDS::CompressionMethod::WaveletNormalizeBlockLossless)) supportedCompressionMethods += ", WaveletNormalizeBlockLossless";

//connection options
  options.add_option("", "", "urls", "Urls with vendor specific protocol.", cxxopts::value<std::vector<std::string>>(urlarg), "<string>");
  options.add_option("", "s", "source-connection", "Vendor specific connection string.", cxxopts::value<std::string>(sourceConnection), "<string>");
  options.add_option("", "d", "destination-connection", "Vendor specific connection string.", cxxopts::value<std::string>(destinationConnection), "<string>");

  options.add_option("", "", "compression-method", std::string("Compression method. Supported compression methods are: ") + supportedCompressionMethods + ".", cxxopts::value<std::string>(compressionMethodString), "<string>");
  options.add_option("", "", "tolerance", "This parameter specifies the compression tolerance when using the wavelet compression method. This value is the maximum deviation from the original data value when the data is converted to 8-bit using the value range. A value of 1 means the maximum allowable loss is the same as quantizing to 8-bit (but the average loss will be much much lower than quantizing to 8-bit). It is not a good idea to directly relate the tolerance to the quality of the compressed data, as the average loss will in general be an order of magnitude lower than the allowable loss.", cxxopts::value<float>(compressionTolerance), "<value>");

  options.add_option("", "", "json-output", "Enable json output.", cxxopts::value<bool>(jsonOutput), "");
  options.add_option("", "h", "help", "Print this help information", cxxopts::value<bool>(help), "");
  options.add_option("", "H", "help-connection", "Print help information about the connection string", cxxopts::value<bool>(helpConnection), "");
  options.add_option("", "", "version", "Print version information.", cxxopts::value<bool>(version), "");

  options.parse_positional("urls");

  if (argc == 1)
  {
    OpenVDS::printInfo(jsonOutput, "Args", options.help());
    return EXIT_SUCCESS;
  }

  try
  {
    options.parse(argc, argv);
  }
  catch(cxxopts::OptionParseException &e)
  {
    OpenVDS::printError(jsonOutput, "Args", e.what());
    return EXIT_FAILURE;
  }

  if (help)
  {
    OpenVDS::printInfo(jsonOutput, "Args", options.help());
    return EXIT_SUCCESS;
  }
  if (helpConnection)
  {
    OpenVDS::printInfo(jsonOutput, "Args", GetConnectionHelpString());
    return EXIT_SUCCESS;
  }

  if (version)
  {
    OpenVDS::printVersion(jsonOutput, "VDSCopy");
    return EXIT_SUCCESS;
  }

  if (urlarg.size() != 2)
  {
    OpenVDS::printError(jsonOutput, "Args", "Failed - missing url/vdsfile argument");
    return EXIT_FAILURE;
  }
  
  std::string sourceUrl = urlarg[0];
  std::string destinationUrl = urlarg[1];


  OpenVDS::Error error;

  std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> sourceHandle(nullptr, &OpenVDS::Close);

  if(OpenVDS::IsSupportedProtocol(sourceUrl))
  {
    sourceHandle.reset(OpenVDS::Open(sourceUrl, sourceConnection, error));
  }
  else
  {
    sourceHandle.reset(OpenVDS::Open(OpenVDS::VDSFileOpenOptions(sourceUrl), error));
  }

  if(error.code != 0)
  {
    OpenVDS::printError(jsonOutput, "VDS", fmt::format("Could not open VDS {}", sourceUrl), error.string);
    return EXIT_FAILURE;
  }

  auto layout = OpenVDS::GetLayout(sourceHandle.get());
  if (!layout)
  {
    OpenVDS::printError(jsonOutput, "VDS", "Internal error, no layout");
    return EXIT_FAILURE;
  }

  if (std::isnan(compressionTolerance))
  {
    compressionTolerance = OpenVDS::GetCompressionTolerance(sourceHandle.get());
  }
  OpenVDS::CompressionMethod compressionMethod = OpenVDS::CompressionMethod::None;

  std::transform(compressionMethodString.begin(), compressionMethodString.end(), compressionMethodString.begin(), asciitolower);

  if(compressionMethodString.empty()) compressionMethod = OpenVDS::GetCompressionMethod(sourceHandle.get());
  else if(compressionMethodString == "none")                          compressionMethod = OpenVDS::CompressionMethod::None;
  else if(compressionMethodString == "wavelet")                       compressionMethod = OpenVDS::CompressionMethod::Wavelet;
  else if(compressionMethodString == "rle")                           compressionMethod = OpenVDS::CompressionMethod::RLE;
  else if(compressionMethodString == "zip")                           compressionMethod = OpenVDS::CompressionMethod::Zip;
  else if(compressionMethodString == "waveletnormalizeblock")         compressionMethod = OpenVDS::CompressionMethod::WaveletNormalizeBlock;
  else if(compressionMethodString == "waveletlossless")               compressionMethod = OpenVDS::CompressionMethod::WaveletLossless;
  else if(compressionMethodString == "waveletnormalizeblocklossless") compressionMethod = OpenVDS::CompressionMethod::WaveletNormalizeBlockLossless;
  else
  {
    OpenVDS::printError(jsonOutput, "CompressionMethod", "Unknown compression method", compressionMethodString);
    return EXIT_FAILURE;
  }

  if(!OpenVDS::IsCompressionMethodSupported(compressionMethod))
  {
    OpenVDS::printError(jsonOutput, "CompressionMethod", "Unsupported compression method", compressionMethodString);
    return EXIT_FAILURE;
  }

  int dimensionCount = layout->GetDimensionality();
  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.reserve(dimensionCount);
  for (int i = 0; i < dimensionCount; i++)
  {
    axisDescriptors.emplace_back(layout->GetAxisDescriptor(i));
  }

  int channelCount = layout->GetChannelCount();
  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  channelDescriptors.reserve(channelCount);
  for (int i = 0; i < channelCount; i++)
  {
    channelDescriptors.emplace_back(layout->GetChannelDescriptor(i));
  }
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor = layout->GetLayoutDescriptor();


  std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> destinationHandle(nullptr, &OpenVDS::Close);
  destinationHandle.reset(OpenVDS::Create(destinationUrl, destinationConnection, layoutDescriptor, axisDescriptors, channelDescriptors, *layout, compressionMethod, compressionTolerance, error));
  if(error.code != 0)
  {
    OpenVDS::printError(jsonOutput, "VDS", fmt::format("Could not create VDS {}", destinationUrl), error.string);
    return EXIT_FAILURE;
  }
  auto sourceAccessManager = OpenVDS::GetAccessManager(sourceHandle.get());
  auto destinationAccessManager = OpenVDS::GetAccessManager(destinationHandle.get());

  auto sourceAccessorDestroyer = [&sourceAccessManager](OpenVDS::VolumeDataPageAccessor* acc) { if (acc) sourceAccessManager.DestroyVolumeDataPageAccessor(acc); };
  auto destinationAccessorDestroyer = [&destinationAccessManager](OpenVDS::VolumeDataPageAccessor* acc)
  {
    if (!acc)
      return;
    acc->Commit();
    destinationAccessManager.DestroyVolumeDataPageAccessor(acc);
  };

  int64_t totalChunks = 0;
  int64_t doneChunks = 0;
  int percentage = 0;
  for (int lod = 0; lod <= layoutDescriptor.GetLODLevels(); lod++)
  {
    for (int dim = 0; dim <= OpenVDS::DimensionsND::Dimensions_45; dim++)
    {
      for (int channel = 0; channel < channelCount; channel++)
      {
        if (sourceAccessManager.GetVDSProduceStatus(OpenVDS::DimensionsND(dim), lod, channel) == OpenVDS::VDSProduceStatus::Normal)
        {
          totalChunks += sourceAccessManager.GetVDSChunkCount(OpenVDS::DimensionsND(dim), lod, channel);
        }
      }
    }
  }

  OpenVDS::printInfo(jsonOutput, destinationUrl, fmt::format("Copying {} to {}. Total chunks to copy is {}", sourceUrl, destinationUrl, totalChunks));
  if (!jsonOutput)
    fmt::print(stdout, "\n");
  ThreadPool threadPool(16);

  for (int lod = 0; lod <= layoutDescriptor.GetLODLevels(); lod++)
  {
    for (int dim = 0; dim <= OpenVDS::DimensionsND::Dimensions_45; dim++)
    {
      std::vector<std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(sourceAccessorDestroyer)>> sourceAccessors;
      sourceAccessors.reserve(channelCount);
      std::vector<std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(destinationAccessorDestroyer)>> destinationAccessors;
      destinationAccessors.reserve(channelCount);
      for (int channel = 0; channel < channelCount; channel++)
      {
        if (sourceAccessManager.GetVDSProduceStatus(OpenVDS::DimensionsND(dim), lod, channel) == OpenVDS::VDSProduceStatus::Normal)
        {
          sourceAccessors.emplace_back(std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(sourceAccessorDestroyer)>(sourceAccessManager.CreateVolumeDataPageAccessor(OpenVDS::DimensionsND(dim), lod, channel, OpenVDS::VolumeDataAccessManager::maxPagesDefault, OpenVDS::VolumeDataPageAccessor::AccessMode_ReadOnly), sourceAccessorDestroyer));
          destinationAccessors.emplace_back(std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(destinationAccessorDestroyer)>(destinationAccessManager.CreateVolumeDataPageAccessor(OpenVDS::DimensionsND(dim), lod, channel, OpenVDS::VolumeDataAccessManager::maxPagesDefault, OpenVDS::VolumeDataPageAccessor::AccessMode_Create), destinationAccessorDestroyer));
        }
        else
        {
          sourceAccessors.emplace_back(std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(sourceAccessorDestroyer)>(nullptr, sourceAccessorDestroyer));
          destinationAccessors.emplace_back(std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(destinationAccessorDestroyer)>(nullptr, destinationAccessorDestroyer));
        }
      }

      if (!sourceAccessors[0])
        continue;

      std::vector<std::future<CopyError>> futures[2];
      futures[0].reserve(32);
      futures[1].reserve(32);
      bool futureBuffer = false;
      CopyError copyError;
      for (int64_t chunk = 0; chunk < sourceAccessors[0]->GetChunkCount(); chunk++)
      {
        for (int channel = 0; channel < channelCount; channel++)
        {
          if (sourceAccessors[channel])
          {
            int64_t mappedChunk = sourceAccessors[channel]->GetMappedChunkIndex(chunk);
            if (sourceAccessors[channel]->GetPrimaryChannelChunkIndex(mappedChunk) == chunk)
            {
              if (futures[futureBuffer].size() == futures[futureBuffer].capacity())
              {
                futureBuffer = !futureBuffer;
                if (!flushFutureBufer(futures[futureBuffer], jsonOutput, totalChunks, doneChunks, percentage, copyError))
                {
                    if (!jsonOutput)
                      fprintf(stderr, "\n");
                    OpenVDS::printError(jsonOutput, "Failed to copy chunk ", copyError.message);
                    return copyError.code;
                }
              }
              futures[futureBuffer].emplace_back(threadPool.Enqueue([&sourceAccessors, channel, mappedChunk, &destinationAccessors, &channelDescriptors, lod, &layout]
              {
                CopyError retError;
                auto sourcePage = sourceAccessors[channel]->ReadPage(mappedChunk);
                auto error = sourcePage->GetError();
                if (error.errorCode)
                {
                  retError.code = error.errorCode;
                  retError.message = error.message;
                  return retError;
                }
                auto destinationPage = destinationAccessors[channel]->CreatePage(mappedChunk);
                int sourcePitch[6];
                auto sourceBuffer = sourcePage->GetBuffer(sourcePitch);
                int destinationPitch[6];
                auto destinationBuffer = destinationPage->GetWritableBuffer(destinationPitch);
                auto maxPitch = std::distance(sourcePitch, std::max_element(sourcePitch, &sourcePitch[6]));
                int pageMin[6];
                int pageMax[6];
                sourcePage->GetMinMax(pageMin, pageMax);
                int formatSize = GetVoxelFormatByteSize(channelDescriptors[channel].GetFormat());
                int bufferSize = sourcePitch[maxPitch] * OpenVDS::GetLODSize(pageMin[maxPitch], pageMax[maxPitch], lod) * formatSize * layout->GetChannelComponents(channel);
                assert(memcmp(sourcePitch, destinationPitch, sizeof(sourcePitch)) == 0);
                memcpy(destinationBuffer, sourceBuffer, bufferSize);
                sourcePage->Release();
                destinationPage->Release();
                return retError;
              }));
            }
          }
        }
      }
      for (auto& futuesInBuffer : futures)
      {
        if (!flushFutureBufer(futuesInBuffer, jsonOutput, totalChunks, doneChunks, percentage, copyError))
        {
          if (!jsonOutput)
            fprintf(stderr, "\n");
          OpenVDS::printError(jsonOutput, "Failed to copy chunk ", copyError.message);
          return copyError.code;
        }
      }
    }
  }

  if (!jsonOutput)
  {
    printPercentage(100.0f);
    fprintf(stdout, "\n");
  }

  OpenVDS::printInfo(jsonOutput, destinationUrl, fmt::format("Successfully copied {} to {}", sourceUrl, destinationUrl));
  return EXIT_SUCCESS;
}
