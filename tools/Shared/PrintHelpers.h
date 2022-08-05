#ifndef PRINT_HELPERS_H
#define PRINT_HELPERS_H

#include <json/json.h>
#include <fmt/format.h>
#include <OpenVDS/OpenVDS.h>
#include <mutex>

namespace OpenVDS
{

struct OutputPrinter
{
  bool json;
  bool printingPercentage;
  std::mutex mutex;
  LogLevel logLevel;
  LogHandler logHandler;

  struct PrintingPercentageGuard
  {
    PrintingPercentageGuard(OutputPrinter* printer)
      : lock(printer->mutex)
    {
      if (printer->printingPercentage)
      {
        fmt::print(stdout, "\n");
        printer->printingPercentage = false;
      }
    }
    std::unique_lock<std::mutex> lock;
  };

  OutputPrinter(bool json, LogLevel logLevel)
    : json(json)
    , printingPercentage(false)
    , logLevel(logLevel)
  {
    logHandler.userHandle = this;
    logHandler.callback = [](LogLevel level, const char* message, size_t messageSize, void* userHandle)
    {
      auto* self = static_cast<OutputPrinter*>(userHandle);
      PrintingPercentageGuard guard(self);
      std::string messageStr(message, messageSize);
      switch (level)
      {
      case LogLevel::None:
        return;
      case LogLevel::Error:
        self->printErrorUnguarded("OpenVDS", messageStr);
        break;
      case LogLevel::Warning:
        self->printWarningUnguarded("OpenVDS", messageStr);
        break;
      case LogLevel::Info:
      case LogLevel::Trace:
        self->printInfoUnguarded("OpenVDS", messageStr);
      }
    };
  }
  static LogLevel getLogLevel(bool disableWarning, bool disableInfo)
  {
    if (disableWarning)
      return LogLevel::Error;
    if (disableInfo)
      return LogLevel::Warning;
    return LogLevel::Info;
  }

  void printVersion(const std::string& name);
  void printPercentage(double);
  void printNewLine(OpenVDS::LogLevel threshold);
  void printInfo(const std::string title, const std::string& str);
  void printInfoUnguarded(const std::string title, const std::string& str);
  void printInfo(const std::string title, const std::string& str, const std::string& value);
  void printWarning(const std::string& title, const std::string& str);
  void printWarningUnguarded(const std::string& title, const std::string& str);
  void printWarning(const std::string& title, const std::string& message, const std::string& value, const std::string& systemError);
  void printError(const std::string& title, const std::string& str);
  void printErrorUnguarded(const std::string& title, const std::string& str);
  void printError(const std::string& title, const std::string& message, const std::string& value);
  void printError(const std::string& title, const std::string& message, const std::string& value, const std::string& systemError);
};

inline void OutputPrinter::printVersion(const std::string &name)
{
  if (logLevel < LogLevel::Info)
    return;
  PrintingPercentageGuard guard(this);
  if (json)
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

inline void OutputPrinter::printPercentage(double percentage)
{
  if (json || logLevel < OpenVDS::LogLevel::Info)
    return;
  std::unique_lock<std::mutex> lock(mutex);
  printingPercentage = true;
  fmt::print(stdout, "\r {0:5.2f} % Done.", percentage);
  fflush(stdout);
}

inline void OutputPrinter::printNewLine(OpenVDS::LogLevel threshold)
{
  if (logLevel < threshold || json)
    return;
  PrintingPercentageGuard guard(this);
  fputc('\n', stdout);
}

inline void OutputPrinter::printInfo(const std::string title, const std::string& str)
{
  if (logLevel < LogLevel::Info)
    return;
  PrintingPercentageGuard guard(this);
  printInfoUnguarded(title, str);
}
inline void OutputPrinter::printInfoUnguarded(const std::string title, const std::string &str)
{
  if (json)
  {
    Json::Value valueObj;
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
    fmt::print(stdout, "{}\n", str);
  }
}

inline void OutputPrinter::printInfo(const std::string title, const std::string &str, const std::string &value)
{
  if (logLevel < LogLevel::Info)
    return;
  PrintingPercentageGuard guard(this);
  if (json)
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

inline void OutputPrinter::printWarning(const std::string& title, const std::string& str)
{
  if (logLevel < LogLevel::Warning)
    return;
  PrintingPercentageGuard guard(this);
  printWarningUnguarded(title, str);
}
inline void OutputPrinter::printWarningUnguarded(const std::string &title, const std::string& str)
{
  if (json)
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

inline void OutputPrinter::printWarning(const std::string& title, const std::string& message, const std::string& value, const std::string &systemError)
{
  if (logLevel < LogLevel::Warning)
    return;
  PrintingPercentageGuard guard(this);
  if (json)
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

inline void OutputPrinter::printError(const std::string& title, const std::string& str)
{
  if (logLevel < LogLevel::Error)
    return;
  PrintingPercentageGuard guard(this);
  printErrorUnguarded(title, str);
}
inline void OutputPrinter::printErrorUnguarded(const std::string &title, const std::string& str)
{
  if (json)
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

inline void OutputPrinter::printError(const std::string& title, const std::string& message, const std::string& value)
{
  if (logLevel < LogLevel::Error)
    return;
  PrintingPercentageGuard guard(this);
  if (json)
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

inline void OutputPrinter::printError(const std::string& title, const std::string& message, const std::string& value, const std::string &systemError)
{
  if (logLevel < LogLevel::Error)
    return;
  PrintingPercentageGuard guard(this);
  if (json)
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
struct PrintWarningContext
{
  Json::Value arrayAcc;
  std::string title;
  std::string fatalMsg;
  OutputPrinter &outputPrinter;
  bool printForceMessage;
  PrintWarningContext(OutputPrinter &outputPrinter, const std::string& title, bool printForceMessage, const std::string fatalMsg)
    : title(title)
    , outputPrinter(outputPrinter)
  {

  }
  ~PrintWarningContext()
  {
    if (outputPrinter.logLevel == LogLevel::None)
      return;
    OutputPrinter::PrintingPercentageGuard guard(&outputPrinter);
    if (outputPrinter.json)
    {
      Json::Value root;
      if (printForceMessage)
      {
        root["error"] = arrayAcc;
        root["info"] = fatalMsg;
      }
      else
      {
        root["warning"] = arrayAcc;
      }
      Json::StreamWriterBuilder wbuilder;
      wbuilder["indentation"] = "  ";
      std::string document = Json::writeString(wbuilder, root);
      fmt::print(stdout, "{}\n", document);
    }
    else
    {
      if (printForceMessage)
        outputPrinter.printErrorUnguarded("VDS", fatalMsg);
    }
  }

  void addWarning(const std::string& message, const std::string& value, const std::string& systemError)
  {
    if (outputPrinter.json)
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
      OutputPrinter::PrintingPercentageGuard guard(&outputPrinter);
      fmt::print(stderr, "[{}] {}: {}\n", message, value, systemError);
    }
  }
};
}

#endif
