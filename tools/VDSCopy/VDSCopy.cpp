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

std::string getCompressionMethodString(OpenVDS::CompressionMethod compressionMethod)
{
  static const char* names[] = {
  "None",
  "Wavelet",
  "RLE",
  "Zip",
  "WaveletNormalizeBlock",
  "WaveletLossless",
  "WaveletNormalizeBlockLossless"
  };
  return names[int(compressionMethod)];
}
inline char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  return in;
}

struct CopyError
{
  int code = 0;
  std::string message;
};

static void printProgress(OpenVDS::OutputPrinter &outputPrinter, int64_t totalChunks, int64_t doneChunks, int& percentage)
{
  int newPercentage = int(doneChunks * 10000 / totalChunks);
  if (newPercentage != percentage)
  {
    percentage = newPercentage;
    double output_percentage = double(percentage) / 100.0;
    outputPrinter.printPercentage(output_percentage);
  }
}

static void printError(OpenVDS::OutputPrinter &outputPrinter, const OpenVDS::Error &error, bool ignoreErrors, bool &error_encountered, bool &keep_processing)
{
  error_encountered = true;
  if (!ignoreErrors)
    keep_processing = false;
  auto message = fmt::format("{}: {}", error.code, error.string);
  outputPrinter.printError("Failed to copy chunk ", message);
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

  bool resumeMode = false;
  int flushFrequency = 60;

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

  options.add_option("", "", "resume", "Resume a copy of a previous partial copy", cxxopts::value<bool>(resumeMode), "");
  options.add_option("", "", "flush-frequency", std::string("Flush frequency in seconds. VDSCopy can resume imports at flush checkpoints. 0 (zero) results in never flushing. Default is 60"), cxxopts::value<int>(flushFrequency), "<value>");

  options.add_option("", "f", "force", "Force/ignore errors", cxxopts::value<bool>(ignoreErrors), "");
  options.add_option("", "q", "quiet", "Disable info level output.", cxxopts::value<bool>(disableInfo), "");
  options.add_option("", "Q", "very-quiet", "Disable warning level output.", cxxopts::value<bool>(disableWarning), "");
  options.add_option("", "", "json-output", "Enable json output.", cxxopts::value<bool>(useJsonOutput), "");
  options.add_option("", "h", "help", "Print this help information", cxxopts::value<bool>(help), "");
  options.add_option("", "H", "help-connection", "Print help information about the connection string", cxxopts::value<bool>(helpConnection), "");
  options.add_option("", "", "version", "Print version information.", cxxopts::value<bool>(version), "");

  options.parse_positional("urls");

  OpenVDS::OutputPrinter outputPrinter(useJsonOutput, OpenVDS::OutputPrinter::getLogLevel(disableWarning, disableInfo));

  if (argc == 1)
  {
    outputPrinter.printInfo("Args", options.help());
    return EXIT_SUCCESS;
  }

  try
  {
    options.parse(argc, argv);
  }
  catch(cxxopts::OptionParseException &e)
  {
    outputPrinter.printError("Args", e.what());
    return EXIT_FAILURE;
  }
  
  if (help)
  {
    outputPrinter.printInfo("Args", options.help());
    return EXIT_SUCCESS;
  }
  if (helpConnection)
  {
    outputPrinter.printInfo("Args", GetConnectionHelpString());
    return EXIT_SUCCESS;
  }

  if (version)
  {
    outputPrinter.printVersion("VDSCopy");
    return EXIT_SUCCESS;
  }

  if (urlarg.size() != 2)
  {
    outputPrinter.printError("Args", "Failed - missing url/vdsfile argument");
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
    outputPrinter.printError("VDS", fmt::format("Could not open VDS {}", sourceUrl), error.string);
    return EXIT_FAILURE;
  }

  auto layout = OpenVDS::GetLayout(sourceHandle);
  if (!layout)
  {
    outputPrinter.printError("VDS", "Internal error, no layout");
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
    outputPrinter.printError("CompressionMethod", "Unknown compression method", compressionMethodString);
    return EXIT_FAILURE;
  }
  if (!OpenVDS::IsCompressionMethodSupported(compressionMethod))
  {
    outputPrinter.printError("CompressionMethod", "Unsupported compression method", compressionMethodString);
    return EXIT_FAILURE;
  }

  int channelCount = layout->GetChannelCount();
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor = layout->GetLayoutDescriptor();

  OpenVDS::ScopedVDSHandle destinationHandle;
  if (resumeMode)
  {
    destinationHandle = OpenVDS::Open(destinationUrl, destinationConnection, error);
    if (error.code)
    {
      outputPrinter.printError("VDS", fmt::format("Could not open VDS {}", destinationUrl), error.string);
      return EXIT_FAILURE;
    }
    if (compressionMethod != OpenVDS::GetCompressionMethod(destinationHandle) && !compressionMethodString.empty())
    {
      outputPrinter.printError("VDS", fmt::format("Resumed VDS does not match specified compression method. Opened destination: {}, Requested: {}",
        getCompressionMethodString(OpenVDS::GetCompressionMethod(destinationHandle)), getCompressionMethodString(compressionMethod)));
      return EXIT_FAILURE;
    }
    if (OpenVDS::GetLayout(destinationHandle)->GetLayoutHash() != layout->GetLayoutHash())
    {
      outputPrinter.printError("VDS", "Layout hashes are not matching", "Resuming copy where layout hashes are not matching is not permitted.");
      if (!ignoreErrors)
        return EXIT_FAILURE;
    }
  }
  else
  {
    int dimensionCount = layout->GetDimensionality();
    std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
    axisDescriptors.reserve(dimensionCount);
    for (int i = 0; i < dimensionCount; i++)
    {
      axisDescriptors.emplace_back(layout->GetAxisDescriptor(i));
    }

    std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
    channelDescriptors.reserve(channelCount);
    for (int i = 0; i < channelCount; i++)
    {
      channelDescriptors.emplace_back(layout->GetChannelDescriptor(i));
    }

    destinationHandle = OpenVDS::Create(destinationUrl, destinationConnection, layoutDescriptor, axisDescriptors, channelDescriptors, *layout, compressionMethod, compressionTolerance, error);
    if (error.code != 0)
    {
      outputPrinter.printError("VDS", fmt::format("Could not create VDS {}", destinationUrl), error.string);
      return EXIT_FAILURE;
    }

  }

  outputPrinter.printVersion("VDSCopy");

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
    outputPrinter.printInfo("Data degradation", "Copying between lossy compressed datasets will lead to a slight data degradation.");
  }

  outputPrinter.printInfo(destinationUrl, fmt::format("Copying {} to {}. Total chunks to copy is {}", sourceUrl, destinationUrl, totalChunks));
  bool error_encountered = false;
  bool keep_processing = true;

  if (flushFrequency == 0)
  {
    flushFrequency = std::numeric_limits<decltype(flushFrequency)>::max();
  }
  auto last_flush = std::chrono::steady_clock::now();

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
          destinationAccessors.emplace_back(std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(destinationAccessorDestroyer)>(destinationAccessManager.CreateVolumeDataPageAccessor(OpenVDS::DimensionsND(dim), lod, channel, OpenVDS::VolumeDataAccessManager::maxPagesDefault, OpenVDS::VolumeDataPageAccessor::AccessMode_CreateWithoutLODGeneration), destinationAccessorDestroyer));
        }
        else
        {
          sourceAccessors.emplace_back(std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(sourceAccessorDestroyer)>(nullptr, sourceAccessorDestroyer));
          destinationAccessors.emplace_back(std::unique_ptr<OpenVDS::VolumeDataPageAccessor, decltype(destinationAccessorDestroyer)>(nullptr, destinationAccessorDestroyer));
        }
      }

      if (!sourceAccessors[0])
        continue;

      for (int64_t chunk = 0; chunk < sourceAccessors[0]->GetChunkCount() && keep_processing; chunk++)
      {
        if (chunk % 10 == 0)
        {
          auto now = std::chrono::steady_clock::now();
          if (now - last_flush > std::chrono::seconds(flushFrequency))
          {
            last_flush = now;
            destinationAccessManager.Flush(error);
            if (error.code)
            {
              printError(outputPrinter, error, ignoreErrors, error_encountered, keep_processing);
              break;
            }
          }
        }
        for (int channel = 0; channel < channelCount && keep_processing; channel++)
        {
          if (sourceAccessors[channel])
          {
            int64_t mappedChunk = sourceAccessors[channel]->GetMappedChunkIndex(chunk);
            if (sourceAccessors[channel]->GetPrimaryChannelChunkIndex(mappedChunk) == chunk)
            {
              destinationAccessors[channel]->CopyPage(mappedChunk, *sourceAccessors[channel]);
              doneChunks++;
              printProgress(outputPrinter, totalChunks, doneChunks, percentage);
            }
          }
        }
      }
    }
  }
  destinationAccessManager.Flush(error);
  if (error.code)
  {
    printError(outputPrinter, error, ignoreErrors, error_encountered, keep_processing);
  }

  if (keep_processing)
  {
    outputPrinter.printPercentage(100.0f);
    if (error_encountered)
      outputPrinter.printInfo(destinationUrl, fmt::format("Copied with errors: {} to {}", sourceUrl, destinationUrl));
    else
      outputPrinter.printInfo(destinationUrl, fmt::format("Successfully copied {} to {}", sourceUrl, destinationUrl));
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
