#include <fmt/printf.h>

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataLayout.h>

#include <cxxopts.hpp>
#include <ThreadPool/ThreadPool.h>
#include <PrintHelpers.h>

#include <HelpConnection.h>

#include <assert.h>

#ifndef WIN32
#include <signal.h>
#endif

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

struct CopyError
{
  int code = 0;
  std::string message;
};

static void printProgress(OpenVDS::PrintConfig printConfig, int64_t totalChunks, int64_t doneChunks, int& percentage)
{
  int newPercentage = int(doneChunks * 10000 / totalChunks);
  if (newPercentage != percentage)
  {
    percentage = newPercentage;
    if (!OpenVDS::isJson(printConfig) && OpenVDS::isInfo(printConfig))
    {
      float output_percentage = float(percentage) / 100;
      printPercentage(output_percentage);
    }
  }
}

int main(int argc, char **argv)
{
#ifndef WIN32
  signal(SIGPIPE, SIG_IGN);
#endif
  std::string info = R"help(VDSCopy - A tool for copying a VDS between locations

VDSCopy will create a new dataset in the target location and copy the content of the source.
This might inflict data recompression and a slight data degradation if a lossy compression method is used.

Use -H or see online documentation for connection string parameters:
http://osdu.pages.community.opengroup.org/platform/domain-data-mgmt-services/seismic/open-vds/connection.html
)help";

  cxxopts::Options options("VDSCopy", info);
  options.positional_help("<source_url> <destination_url>");

  std::vector<std::string> urlarg;
  std::string sourceConnection;
  std::string destinationConnection;
  std::string compressionMethodString;
  float compressionTolerance = std::nanf("nan");

  bool ignoreErrors = false;
  bool useJsonOutput = false;
  bool help = false;
  bool helpConnection = false;
  bool version = false;
  bool disableInfo = false;
  bool disableWarning = false;

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

  options.add_option("", "f", "force", "Force/ignore errors", cxxopts::value<bool>(ignoreErrors), "");
  options.add_option("", "q", "quiet", "Disable info level output.", cxxopts::value<bool>(disableInfo), "");
  options.add_option("", "Q", "very-quiet", "Disable warning level output.", cxxopts::value<bool>(disableWarning), "");
  options.add_option("", "", "json-output", "Enable json output.", cxxopts::value<bool>(useJsonOutput), "");
  options.add_option("", "h", "help", "Print this help information", cxxopts::value<bool>(help), "");
  options.add_option("", "H", "help-connection", "Print help information about the connection string", cxxopts::value<bool>(helpConnection), "");
  options.add_option("", "", "version", "Print version information.", cxxopts::value<bool>(version), "");

  options.parse_positional("urls");

  OpenVDS::PrintConfig printConfig = OpenVDS::createPrintConfig(false, OpenVDS::PrintConfig::Info);

  if (argc == 1)
  {
    OpenVDS::printInfo(printConfig, "Args", options.help());
    return EXIT_SUCCESS;
  }

  try
  {
    options.parse(argc, argv);
  }
  catch(cxxopts::OptionParseException &e)
  {
    OpenVDS::printError(printConfig, "Args", e.what());
    return EXIT_FAILURE;
  }
  
  printConfig = OpenVDS::createPrintConfig(useJsonOutput, disableInfo, disableWarning);

  if (help)
  {
    OpenVDS::printInfo(printConfig, "Args", options.help());
    return EXIT_SUCCESS;
  }
  if (helpConnection)
  {
    OpenVDS::printInfo(printConfig, "Args", GetConnectionHelpString());
    return EXIT_SUCCESS;
  }

  if (version)
  {
    OpenVDS::printVersion(printConfig, "VDSCopy");
    return EXIT_SUCCESS;
  }

  if (urlarg.size() != 2)
  {
    OpenVDS::printError(printConfig, "Args", "Failed - missing url/vdsfile argument");
    return EXIT_FAILURE;
  }
  
  std::string sourceUrl = urlarg[0];
  std::string destinationUrl = urlarg[1];


  OpenVDS::Error error;

  OpenVDS::ScopedVDSHandle sourceHandle;

  if(OpenVDS::IsSupportedProtocol(sourceUrl))
  {
    sourceHandle = OpenVDS::Open(sourceUrl, sourceConnection, error);
  }
  else
  {
    sourceHandle = OpenVDS::Open(OpenVDS::VDSFileOpenOptions(sourceUrl), error);
  }

  if(error.code != 0)
  {
    OpenVDS::printError(printConfig, "VDS", fmt::format("Could not open VDS {}", sourceUrl), error.string);
    return EXIT_FAILURE;
  }

  auto layout = OpenVDS::GetLayout(sourceHandle);
  if (!layout)
  {
    OpenVDS::printError(printConfig, "VDS", "Internal error, no layout");
    return EXIT_FAILURE;
  }

  if (std::isnan(compressionTolerance))
  {
    compressionTolerance = OpenVDS::GetCompressionTolerance(sourceHandle);
  }
  OpenVDS::CompressionMethod compressionMethod = OpenVDS::CompressionMethod::None;

  std::transform(compressionMethodString.begin(), compressionMethodString.end(), compressionMethodString.begin(), asciitolower);

  if(compressionMethodString.empty()) compressionMethod = OpenVDS::GetCompressionMethod(sourceHandle);
  else if(compressionMethodString == "none")                          compressionMethod = OpenVDS::CompressionMethod::None;
  else if(compressionMethodString == "wavelet")                       compressionMethod = OpenVDS::CompressionMethod::Wavelet;
  else if(compressionMethodString == "rle")                           compressionMethod = OpenVDS::CompressionMethod::RLE;
  else if(compressionMethodString == "zip")                           compressionMethod = OpenVDS::CompressionMethod::Zip;
  else if(compressionMethodString == "waveletnormalizeblock")         compressionMethod = OpenVDS::CompressionMethod::WaveletNormalizeBlock;
  else if(compressionMethodString == "waveletlossless")               compressionMethod = OpenVDS::CompressionMethod::WaveletLossless;
  else if(compressionMethodString == "waveletnormalizeblocklossless") compressionMethod = OpenVDS::CompressionMethod::WaveletNormalizeBlockLossless;
  else
  {
    OpenVDS::printError(printConfig, "CompressionMethod", "Unknown compression method", compressionMethodString);
    return EXIT_FAILURE;
  }
  if (!OpenVDS::IsCompressionMethodSupported(compressionMethod))
  {
    OpenVDS::printError(printConfig, "CompressionMethod", "Unsupported compression method", compressionMethodString);
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


  OpenVDS::ScopedVDSHandle destinationHandle;
  destinationHandle = OpenVDS::Create(destinationUrl, destinationConnection, layoutDescriptor, axisDescriptors, channelDescriptors, *layout, compressionMethod, compressionTolerance, error);
  if(error.code != 0)
  {
    OpenVDS::printError(printConfig, "VDS", fmt::format("Could not create VDS {}", destinationUrl), error.string);
    return EXIT_FAILURE;
  }

  OpenVDS::printVersion(printConfig, "VDSCopy");

  auto sourceAccessManager = OpenVDS::GetAccessManager(sourceHandle);
  auto destinationAccessManager = OpenVDS::GetAccessManager(destinationHandle);

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

  auto sourceCompressionMethod = OpenVDS::GetCompressionMethod(sourceHandle);
  auto destinationCompressionMethod = OpenVDS::GetCompressionMethod(destinationHandle);

  if ((sourceCompressionMethod == OpenVDS::CompressionMethod::Wavelet || sourceCompressionMethod == OpenVDS::CompressionMethod::WaveletNormalizeBlock)
    && (destinationCompressionMethod == OpenVDS::CompressionMethod::Wavelet || destinationCompressionMethod == OpenVDS::CompressionMethod::WaveletNormalizeBlock)
    && (OpenVDS::GetCompressionTolerance(sourceHandle) != OpenVDS::GetCompressionTolerance(destinationHandle)))
  {
    OpenVDS::printInfo(printConfig, "Data degradation", "Copying between lossy compressed datasets will lead to a slight data degradation.");
  }

  OpenVDS::printInfo(printConfig, destinationUrl, fmt::format("Copying {} to {}. Total chunks to copy is {}", sourceUrl, destinationUrl, totalChunks));
  if (!OpenVDS::isJson(printConfig) && OpenVDS::isInfo(printConfig))
    fmt::print(stdout, "\n");
  bool error_encountered = false;
  bool keep_processing = true;
  for (int lod = 0; lod <= layoutDescriptor.GetLODLevels() && keep_processing; lod++)
  {
    for (int dim = 0; dim <= OpenVDS::DimensionsND::Dimensions_45 && keep_processing; dim++)
    {
      std::vector<std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(sourceAccessorDestroyer)>> sourceAccessors;
      sourceAccessors.reserve(channelCount);
      std::vector<std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(destinationAccessorDestroyer)>> destinationAccessors;
      destinationAccessors.reserve(channelCount);
      for (int channel = 0; channel < channelCount && keep_processing; channel++)
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

      CopyError copyError;
      for (int64_t chunk = 0; chunk < sourceAccessors[0]->GetChunkCount() && keep_processing; chunk++)
      {
        for (int channel = 0; channel < channelCount && keep_processing; channel++)
        {
          if (sourceAccessors[channel])
          {
            int64_t mappedChunk = sourceAccessors[channel]->GetMappedChunkIndex(chunk);
            if (sourceAccessors[channel]->GetPrimaryChannelChunkIndex(mappedChunk) == chunk)
            {
              destinationAccessors[channel]->CopyPage(mappedChunk, *sourceAccessors[channel]);
              doneChunks++;
              printProgress(printConfig, totalChunks, doneChunks, percentage);
              int errorCount = destinationAccessManager.UploadErrorCount();
              for (int errorIndex = 0; errorIndex < errorCount; errorIndex++)
              {
                const char* objectId;
                int32_t errorCode;
                const char* errorString;
                destinationAccessManager.GetCurrentUploadError(&objectId, &errorCode, &errorString);
                if (errorIndex == 0)
                {
                  copyError.message = fmt::format("{}: {}", objectId, errorString);
                  copyError.code = errorCode;
                  OpenVDS::printError(printConfig, "Failed to copy chunk ", copyError.message);
                }
                error_encountered = true;
                if (!ignoreErrors)
                  keep_processing = false;
              }
            }
          }
        }
      }
    }
  }

  if (keep_processing)
  {
    if (!OpenVDS::isJson(printConfig) && OpenVDS::isInfo(printConfig))
    {
      printPercentage(100.0f);
      fprintf(stdout, "\n");
    }
    if (error_encountered)
      OpenVDS::printInfo(printConfig, destinationUrl, fmt::format("Copied with errors: {} to {}", sourceUrl, destinationUrl));
    else
      OpenVDS::printInfo(printConfig, destinationUrl, fmt::format("Successfully copied {} to {}", sourceUrl, destinationUrl));
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
