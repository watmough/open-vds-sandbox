/****************************************************************************
** Copyright 2021 The Open Group
** Copyright 2021 Bluware, Inc.
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
#include <VDS/VDS.h>
#include <VDS/VolumeDataAccessManagerImpl.h>
#include <VDS/VolumeDataStore.h>
#include <VDS/VolumeDataStoreIOManager.h>
#include <IO/IOManagerTransformer.h>
#include <utils/FacadeIOManager.h>
#include <OpenVDS/Vector.h>

#include <fmt/printf.h>

#if defined(_MSC_VER) && _MSC_VER <= 1900
#pragma warning( push )
#pragma warning( disable : 4800 )
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/cast.h>

#if defined(_MSC_VER) && _MSC_VER <= 1900
#pragma warning( pop )
#endif

namespace py = pybind11;

struct IOManagerErrorDataStore
{
  std::map<std::string, Object> backendData;
  std::mutex mutex;
  void setError(const std::string& objectName, const std::vector<std::pair<std::string, std::string>>& metadata, const std::vector<uint8_t>& data, int error_code, const std::string& error_string)
  {
    std::unique_lock<std::mutex> lock(mutex);
    auto& d = backendData[objectName];
    d.metaHeader = metadata;
    d.data = data;
    d.error.code = error_code;
    d.error.string = error_string;
  }
  void close()
  {
    OpenVDS::SetIoManagerTransformer(std::function<OpenVDS::IOManager* (OpenVDS::IOManager*)>());
    backendData.clear();
  }
};

static std::unique_ptr<IOManagerErrorDataStore> errorDataStore;

class IOManagerPythonFacade : public OpenVDS::IOManager
{
public:
  IOManagerPythonFacade(OpenVDS::IOManager* backend, IOManagerErrorDataStore* errorDataStore)
    : IOManager(backend->connectionType())
    , facade(backend, errorDataStore->backendData, errorDataStore->mutex)
  {
  }
  std::shared_ptr<OpenVDS::Request> ReadObjectInfo(const std::string& objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler) override
  {
    return facade.ReadObjectInfo(objectName, handler);
  }
  std::shared_ptr<OpenVDS::Request> ReadObject(const std::string& objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler, const OpenVDS::IORange& range = OpenVDS::IORange()) override
  {
    return facade.ReadObject(objectName, handler, range);
  }
  std::shared_ptr<OpenVDS::Request> WriteObject(const std::string& objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const OpenVDS::Request& request, const OpenVDS::Error& error)> completedCallback = nullptr) override
  {
    return facade.WriteObject(objectName, contentDispostionFilename, contentType, metadataHeader, data, completedCallback);
  }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4717)
#endif
  bool Close(uint64_t serializedSize, uint64_t chunkCount, OpenVDS::Error &error) override { return facade.Close(serializedSize, chunkCount, error); }
#ifdef _MSC_VER
#pragma warning(pop)
#endif


  std::unique_ptr<OpenVDS::IOManager> deleter;
  IOManagerFacadeUtil facade;
};


IOManagerErrorDataStore* enableIoError()
{
  auto ret = new IOManagerErrorDataStore();
  auto func = [ret](OpenVDS::IOManager* backend)
  {
    return new IOManagerPythonFacade(backend, ret);
  };
  OpenVDS::SetIoManagerTransformer(func);
  return ret;
}

namespace pybind11
{
namespace detail
{

}
}

PYBIND11_MODULE(openvds_python_test, m) {
  m.def("enableIoError", []() { return enableIoError(); }, py::call_guard<py::gil_scoped_release>());

  py::class_<IOManagerErrorDataStore> errorDataStore(m, "IOManagerErrorDataStore");
  errorDataStore.def("__enter__", [](IOManagerErrorDataStore *self) { return self;});
  errorDataStore.def("__exit__", [](IOManagerErrorDataStore* self, py::args) { self->close(); });
  errorDataStore.def("setError", [](IOManagerErrorDataStore* self, const std::string& objectName, const std::vector<std::pair<std::string, std::string>>& metadata, const std::vector<uint8_t>& data, int error_code, const std::string& error_string)
  {
    self->setError(objectName, metadata, data, error_code, error_string);
  } , py::call_guard<py::gil_scoped_release>());
}
