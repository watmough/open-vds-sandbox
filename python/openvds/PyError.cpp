/****************************************************************************
** Copyright 2020 The Open Group
** Copyright 2020 Bluware, Inc.
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

using namespace native;

void
PyError::initModule(py::module& m)
{
//AUTOGEN-BEGIN
  // Error
  py::class_<Error> 
    Error_(m,"Error", OPENVDS_DOCSTRING(Error));

  Error_.def_readwrite("code"                        , &Error::code                   , OPENVDS_DOCSTRING(Error_code));
  Error_.def_readwrite("string"                      , &Error::string                 , OPENVDS_DOCSTRING(Error_string));

//AUTOGEN-END

  Error_.def(py::init<>());
  Error_.def("__repr__", [](native::Error const& self){ std::string tmp = std::to_string(self.code); return std::string("Error(code=") + tmp + ", string='" + self.string + "')"; });

}
