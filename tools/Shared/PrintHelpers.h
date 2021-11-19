#ifndef PRINT_HELPERS_H
#define PRINT_HELPERS_H

#include <json/json.h>
#include <fmt/format.h>
#include <OpenVDS/OpenVDS.h>

namespace OpenVDS
{

enum class PrintConfig
{
  NoOutput = 0,
  Error = 1 << 1,
  Warning = 1 << 2,
  Info = 1 << 3,
  Json = 1 << 4
};

inline bool isNoOutput(PrintConfig config) { return (int)config & (int)PrintConfig::NoOutput; }
inline bool isInfo(PrintConfig config) { return (int)config & (int)PrintConfig::Info; }
inline bool isWarning(PrintConfig config) { return (int)config & (int)PrintConfig::Warning; }
inline bool isError(PrintConfig config) { return (int)config & (int)PrintConfig::Error; }
inline bool isJson(PrintConfig config) { return (int)config & (int)PrintConfig::Json; }

PrintConfig getOutputLevel(bool disableInfo, bool disableWarning)
{
  OpenVDS::PrintConfig outputLevel = OpenVDS::PrintConfig::Info;
  if (disableInfo)
    outputLevel = OpenVDS::PrintConfig::Warning;
  if (disableWarning)
    outputLevel = OpenVDS::PrintConfig::Error;
  return outputLevel;
}

PrintConfig createPrintConfig(bool json, PrintConfig minSeverity)
{
  int n = 0;
  if (json)
    n |= (int)PrintConfig::Json;
  n |= (int(minSeverity) << 1) - 1;
  return (PrintConfig)n;
}

PrintConfig createPrintConfig(bool json, bool disableInfo, bool disableWarning)
{
  return createPrintConfig(json, getOutputLevel(disableInfo, disableWarning));
}

void
printInfo(PrintConfig config, const std::string title, const std::string &str)
{
  if (isNoOutput(config) || !isInfo(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["message"] = str;
    valueObj["title"] = title;
    Json::Value info;
    info["info"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, info);
    fmt::print(stdout, "{}", document);
  }
  else
  {
    fmt::print(stdout, "{}", str);
  }
}

void
printInfo(PrintConfig config, const std::string title, const std::string &str, const std::string &value)
{
  if (isNoOutput(config) || !isInfo(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["value"] = value;
    valueObj["message"] = str;
    valueObj["title"] = title;
    Json::Value info;
    info["info"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, info);
    fmt::print(stdout, "{}\n", document);
  }
  else
  {
    fmt::print(stdout, "{}: {}\n", str, value);
  }
}

void
printVersion(PrintConfig config, const std::string &name)
{
  if (isNoOutput(config) || !isInfo(config))
    return;
  if (isJson(config))
  {
    Json::Value version;
    version["name"] = name;
    version["project"] = OpenVDS::GetOpenVDSName();
    version["version"] = OpenVDS::GetOpenVDSVersion();
    std::string rev = OpenVDS::GetOpenVDSRevision();
    if (rev.size())
    {
      version["revision"] = OpenVDS::GetOpenVDSRevision();
    }
    Json::Value info;
    info["version"] = version;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, info);
    fmt::print(stdout, "{}\n", document);
  }
  else
  {
    std::string rev = OpenVDS::GetOpenVDSRevision();
    if (rev.size())
    {
      fmt::print(stdout, "{} - {} {} - Revision: {}\n", name, OpenVDS::GetOpenVDSName(), OpenVDS::GetOpenVDSVersion(), rev);
    }
    else
    {
      fmt::print(stdout, "{} - {} {}\n", name, OpenVDS::GetOpenVDSName(), OpenVDS::GetOpenVDSVersion());
    }
  }
}

void
printWarning(PrintConfig config, const std::string &title, const std::string& str)
{
  if (isNoOutput(config) || !isWarning(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["message"] = str;
    valueObj["title"] = title;
    Json::Value warning;
    warning["warning"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, warning);
    fmt::print(stdout, "{}\n", document);
  }
  else
  {
    fmt::print(stderr, "[{}]\n", str);
  }
}

void
printWarning(PrintConfig config, const std::string& title, const std::string& message, const std::string& value, const std::string &systemError)
{
  if (isNoOutput(config) || !isWarning(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["message"] = message;
    valueObj["title"] = title;
    valueObj["value"] = value;
    valueObj["error"] = systemError;
    Json::Value warning;
    warning["warning"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, warning);
    fmt::print(stdout, "{}\n", document);
  }
  else
  {
    fmt::print(stderr, "[{}] {}: {}\n", message, value, systemError);
  }
}

void
printError(PrintConfig config, const std::string &title, const std::string& str)
{
  if (isNoOutput(config) || !isError(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["message"] = str;
    valueObj["title"] = title;
    Json::Value error;
    error["error"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, error);
    fmt::print(stdout, "{}\n", document);
  }
  else
  {
    fmt::print(stderr, "[{}]\n", str);
  }
}

void
printError(PrintConfig config, const std::string& title, const std::string& message, const std::string& value)
{
  if (isNoOutput(config) || !isError(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["message"] = message;
    valueObj["title"] = title;
    valueObj["value"] = value;
    Json::Value error;
    error["error"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, error);
    fmt::print(stdout, "{}\n", document);
  }
  else
  {
    fmt::print(stderr, "[{}] {}\n", message, value);
  }
}

void
printError(PrintConfig config, const std::string& title, const std::string& message, const std::string& value, const std::string &systemError)
{
  if (isNoOutput(config) || !isError(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["message"] = message;
    valueObj["title"] = title;
    valueObj["value"] = value;
    valueObj["error"] = systemError;
    Json::Value error;
    error["error"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, error);
    fmt::print(stdout, "{}\n", document);
  }
  else
  {
    fmt::print(stderr, "[{}] {}: {}\n", message, value, systemError);
  }
}

void
printWarning_with_condition_fatal(PrintConfig config, bool fatal, const std::string title, const std::string& value, const std::string& fatal_value)
{
  if (isNoOutput(config))
    return;
  if (isJson(config))
  {
    Json::Value valueObj;
    valueObj["message"] = value;
    valueObj["title"] = title;
    if (fatal & isError(config))
      valueObj["info"] = fatal_value;
    Json::Value root;
    if (fatal & isError(config))
      root["error"] = valueObj;
    else if (isWarning(config))
      root["warning"] = valueObj;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::string document = Json::writeString(wbuilder, root);
    if (isError(config) || isWarning(config))
      fmt::print(stdout, "{}\n", document);
  }
  else
  {
    if (isWarning(config))
      printWarning(config, title, value);
    if(fatal && isError(config))
    {
      printError(config, title, fatal_value);
    }
  }
  if (fatal)
    exit(1);
}

struct PrintWarningContext
{
  Json::Value arrayAcc;
  std::string title;
  std::string fatalMsg;
  PrintConfig config;
  bool fatal;
  PrintWarningContext(PrintConfig config, const std::string& title, bool fatal, const std::string fatalMsg)
    : title(title)
    , config(config)
    , fatal(fatal)
  {

  }
  ~PrintWarningContext()
  {
    if (isNoOutput(config))
      return;
    if (isJson(config))
    {
      Json::Value root;
      if (fatal && isError(config))
      {
        root["error"] = arrayAcc;
        root["info"] = fatalMsg;
      }
      else if (isWarning(config))
      {
        root["warning"] = arrayAcc;
      }
      Json::StreamWriterBuilder wbuilder;
      wbuilder["indentation"] = "  ";
      std::string document = Json::writeString(wbuilder, root);
      if (isError(config) || isWarning(config))
        fmt::print(stdout, "{}\n", document);
    }
    else
    {
      if (fatal)
        printError(config, "VDS", fatalMsg);
    }
    if (fatal)
    {
      exit(EXIT_FAILURE);
    }
  }

  void addWarning(const std::string& message, const std::string& value, const std::string& systemError)
  {
    if (isJson(config))
    {
      Json::Value obj;
      obj["message"] = message;
      obj["title"] = title;
      obj["value"] = value;
      obj["error"] = systemError;
      arrayAcc.append(obj);
    }
    else
    {
      if (isWarning(config))
        fmt::print(stderr, "[{}] {}: {}\n", message, value, systemError);
    }
  }
};

}

#endif
