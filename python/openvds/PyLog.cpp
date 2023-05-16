/****************************************************************************
** Copyright 2023 The Open Group
** Copyright 2023 Bluware, Inc.
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

#include "PyLog.h"

#include <OpenVDS/Log.h>

using namespace native;

void
PyLog::initModule(py::module& m)
{
//AUTOGEN-BEGIN
  py::enum_<LogLevel> 
    LogLevel_(m,"LogLevel", OPENVDS_DOCSTRING(LogLevel));

  LogLevel_.value("None"                        , LogLevel::None                          , OPENVDS_DOCSTRING(LogLevel_None));
  LogLevel_.value("Error"                       , LogLevel::Error                         , OPENVDS_DOCSTRING(LogLevel_Error));
  LogLevel_.value("Warning"                     , LogLevel::Warning                       , OPENVDS_DOCSTRING(LogLevel_Warning));
  LogLevel_.value("Info"                        , LogLevel::Info                          , OPENVDS_DOCSTRING(LogLevel_Info));
  LogLevel_.value("Trace"                       , LogLevel::Trace                         , OPENVDS_DOCSTRING(LogLevel_Trace));

//AUTOGEN-END

}

