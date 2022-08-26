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

#include "PyError.h"

#include <OpenVDS/Error.h>

using namespace native;

void 
PyError::initModule(py::module& m)
{
//AUTOGEN-BEGIN
  // VDSError
  py::class_<VDSError> 
    VDSError_(m,"VDSError", OPENVDS_DOCSTRING(VDSError));

  VDSError_.def(py::init<                              >(), OPENVDS_DOCSTRING(VDSError_VDSError));
  VDSError_.def_readwrite("code"                        , &VDSError::code                , OPENVDS_DOCSTRING(VDSError_code));
  VDSError_.def_readwrite("string"                      , &VDSError::string              , OPENVDS_DOCSTRING(VDSError_string));

//AUTOGEN-END
  VDSError_.def("__repr__", [](native::Error const& self){ std::string tmp = std::to_string(self.code); return std::string("Error(code=") + tmp + ", string='" + self.string + "')"; });

}

