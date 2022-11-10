#ifndef OPENVDS_ERROR_REQUEST_H
#define OPENVDS_ERROR_REQUEST_H

#include "IOManager.h"

namespace OpenVDS
{
class ErrorRequest : public Request
{
public:
  ErrorRequest(const std::string& objectName, const std::string &error) : Request(objectName), m_error(error) {}
  bool WaitForFinish(Error& error) override
  {
    error.code = -1;
    error.string = m_error;
    return false;
  }
  void Cancel() override {}
  std::string m_error;
};

}

#endif