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

#include "PyCoordinateTransformer.h"

using namespace native;
void 
PyCoordinateTransformer::initModule(py::module& m)
{
//AUTOGEN-BEGIN
  // IJKGridDefinition
  py::class_<IJKGridDefinition> 
    IJKGridDefinition_(m,"IJKGridDefinition", OPENVDS_DOCSTRING(IJKGridDefinition));
  IJKGridDefinition_.def(py::init<                              >(), OPENVDS_DOCSTRING(IJKGridDefinition_IJKGridDefinition));
  IJKGridDefinition_.def(py::init<native::DoubleVector3, native::DoubleVector3, native::DoubleVector3, native::DoubleVector3>(), py::arg("origin").none(false), py::arg("iUnitStep").none(false), py::arg("jUnitStep").none(false), py::arg("kUnitStep").none(false), OPENVDS_DOCSTRING(IJKGridDefinition_IJKGridDefinition_2));
  IJKGridDefinition_.def(py::self == py::self);
  IJKGridDefinition_.def_readwrite("origin"                      , &IJKGridDefinition::origin     , OPENVDS_DOCSTRING(IJKGridDefinition_origin));
  IJKGridDefinition_.def_readwrite("iUnitStep"                   , &IJKGridDefinition::iUnitStep  , OPENVDS_DOCSTRING(IJKGridDefinition_iUnitStep));
  IJKGridDefinition_.def_readwrite("jUnitStep"                   , &IJKGridDefinition::jUnitStep  , OPENVDS_DOCSTRING(IJKGridDefinition_jUnitStep));
  IJKGridDefinition_.def_readwrite("kUnitStep"                   , &IJKGridDefinition::kUnitStep  , OPENVDS_DOCSTRING(IJKGridDefinition_kUnitStep));
  // VDSIJKGridDefinition
  py::class_<VDSIJKGridDefinition, IJKGridDefinition> 
    VDSIJKGridDefinition_(m,"VDSIJKGridDefinition", OPENVDS_DOCSTRING(VDSIJKGridDefinition));
  VDSIJKGridDefinition_.def(py::init<                              >(), OPENVDS_DOCSTRING(VDSIJKGridDefinition_VDSIJKGridDefinition));
  VDSIJKGridDefinition_.def(py::init<const native::IJKGridDefinition &, native::IntVector3>(), py::arg("ijkGridDefinition").none(false), py::arg("dimensionMap").none(false), OPENVDS_DOCSTRING(VDSIJKGridDefinition_VDSIJKGridDefinition_2));
  VDSIJKGridDefinition_.def(py::self == py::self);
  VDSIJKGridDefinition_.def_readwrite("dimensionMap"                , &VDSIJKGridDefinition::dimensionMap, OPENVDS_DOCSTRING(VDSIJKGridDefinition_dimensionMap));
//AUTOGEN-END   

// IMPLEMENTED :  // M4
// IMPLEMENTED :  py::class_<M4> 
// IMPLEMENTED :    M4_(m,"M4", OPENVDS_DOCSTRING(M4));
// IMPLEMENTED :  
// IMPLEMENTED :  M4_.def_readwrite("data"                        , &M4::data                      , OPENVDS_DOCSTRING(M4_data));
// IMPLEMENTED :  
// IMPLEMENTED :  m.def("fastInvert"                  , static_cast<void(*)(native::M4 &)>(&fastInvert), py::arg("m").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(fastInvert));

}
