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

#include "PyExceptions.h"

#include <OpenVDS/Exceptions.h>

using namespace native;

#if 1 //PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION == 6
namespace pybind11_old
{
template <typename CppException>
py::exception<CppException> &get_exception_object() {
    static py::exception<CppException> ex;
    return ex;
}

template <typename CppException>
py::exception<CppException> &
register_exception(py::handle scope, const char *name, py::handle base = PyExc_Exception) {
    auto &ex = get_exception_object<CppException>();
    if (!ex) {
        ex = py::exception<CppException>(scope, name, base);
    }

    py::register_exception_translator([](std::exception_ptr p) {
        if (!p) {
            return;
        }
        try {
            std::rethrow_exception(p);
        } catch (const CppException &e) {
            py::set_error(get_exception_object<CppException>(), e.what());
        }
    });
    return ex;
}
}
#define register_exception pybind11_old::register_exception
#else
#define register_exception py::register_exception
#endif

void 
PyExceptions::initModule(py::module& m)
{
//AUTOGEN-BEGIN
//AUTOGEN-END

// IMPLEMENTED : // Exception
// IMPLEMENTED :  py::class_<Exception, exception> 
// IMPLEMENTED :    Exception_(m,"Exception", OPENVDS_DOCSTRING(Exception));
// IMPLEMENTED :
// IMPLEMENTED :  Exception_.def("what"                        , static_cast<const char *(Exception::*)() const>(&Exception::what), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Exception_what));
// IMPLEMENTED :  Exception_.def("getErrorMessage"             , static_cast<const char *(Exception::*)() const>(&Exception::GetErrorMessage), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Exception_GetErrorMessage));
// IMPLEMENTED :  Exception_.def_property_readonly("errorMessage", &Exception::GetErrorMessage, OPENVDS_DOCSTRING(Exception_GetErrorMessage));
register_exception<Exception>(m, "Exception");

// IMPLEMENTED :  // FatalException
// IMPLEMENTED :  py::class_<FatalException, MessageBufferException> 
// IMPLEMENTED :    FatalException_(m,"FatalException", OPENVDS_DOCSTRING(FatalException));
// IMPLEMENTED :  FatalException_.def(py::init<const char *                  >(), py::arg("errorMessage").none(false), OPENVDS_DOCSTRING(FatalException_FatalException));
// IMPLEMENTED :  FatalException_.def(py::init<const native::FatalException &>(), py::arg("other").none(false), OPENVDS_DOCSTRING(FatalException_FatalException_2));
// IMPLEMENTED :  FatalException_.def("operator_assign"             , static_cast<native::FatalException &(FatalException::*)(const native::FatalException &)>(&FatalException::operator=), py::arg("other").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(FatalException_operator_assign));
// IMPLEMENTED :  FatalException_.def("getErrorMessage"             , static_cast<const char *(FatalException::*)() const>(&FatalException::GetErrorMessage), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(FatalException_GetErrorMessage));
// IMPLEMENTED :  FatalException_.def_property_readonly("errorMessage", &FatalException::GetErrorMessage, OPENVDS_DOCSTRING(FatalException_GetErrorMessage));
register_exception<FatalException>(m, "FatalException");

// IMPLEMENTED :  // InvalidOperation
// IMPLEMENTED :  py::class_<InvalidOperation, MessageBufferException> 
// IMPLEMENTED :    InvalidOperation_(m,"InvalidOperation", OPENVDS_DOCSTRING(InvalidOperation));
// IMPLEMENTED :  InvalidOperation_.def(py::init<const char *                  >(), py::arg("errorMessage").none(false), OPENVDS_DOCSTRING(InvalidOperation_InvalidOperation));
// IMPLEMENTED :  InvalidOperation_.def(py::init<const native::InvalidOperation &>(), py::arg("other").none(false), OPENVDS_DOCSTRING(InvalidOperation_InvalidOperation_2));
// IMPLEMENTED :  InvalidOperation_.def("operator_assign"             , static_cast<native::InvalidOperation &(InvalidOperation::*)(const native::InvalidOperation &)>(&InvalidOperation::operator=), py::arg("other").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(InvalidOperation_operator_assign));
// IMPLEMENTED :  InvalidOperation_.def("getErrorMessage"             , static_cast<const char *(InvalidOperation::*)() const>(&InvalidOperation::GetErrorMessage), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(InvalidOperation_GetErrorMessage));
// IMPLEMENTED :  InvalidOperation_.def_property_readonly("errorMessage", &InvalidOperation::GetErrorMessage, OPENVDS_DOCSTRING(InvalidOperation_GetErrorMessage));
register_exception<InvalidOperation>(m, "InvalidOperation");

// IMPLEMENTED :  // InvalidArgument
// IMPLEMENTED :  py::class_<InvalidArgument, MessageBufferException> 
// IMPLEMENTED :    InvalidArgument_(m,"InvalidArgument", OPENVDS_DOCSTRING(InvalidArgument));
// IMPLEMENTED :  InvalidArgument_.def(py::init<const char *, const char *    >(), py::arg("errorMessage").none(false), py::arg("parameterName").none(false), OPENVDS_DOCSTRING(InvalidArgument_InvalidArgument));
// IMPLEMENTED :  InvalidArgument_.def(py::init<const native::InvalidArgument &>(), py::arg("other").none(false), OPENVDS_DOCSTRING(InvalidArgument_InvalidArgument_2));
// IMPLEMENTED :  InvalidArgument_.def("operator_assign"             , static_cast<native::InvalidArgument &(InvalidArgument::*)(const native::InvalidArgument &)>(&InvalidArgument::operator=), py::arg("other").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(InvalidArgument_operator_assign));
// IMPLEMENTED :  InvalidArgument_.def("getErrorMessage"             , static_cast<const char *(InvalidArgument::*)() const>(&InvalidArgument::GetErrorMessage), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(InvalidArgument_GetErrorMessage));
// IMPLEMENTED :  InvalidArgument_.def_property_readonly("errorMessage", &InvalidArgument::GetErrorMessage, OPENVDS_DOCSTRING(InvalidArgument_GetErrorMessage));
// IMPLEMENTED :  InvalidArgument_.def("getParameterName"            , static_cast<const char *(InvalidArgument::*)() const>(&InvalidArgument::GetParameterName), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(InvalidArgument_GetParameterName));
// IMPLEMENTED :  InvalidArgument_.def_property_readonly("parameterName", &InvalidArgument::GetParameterName, OPENVDS_DOCSTRING(InvalidArgument_GetParameterName));
register_exception<InvalidArgument>(m, "InvalidArgument");

// IMPLEMENTED :  // IndexOutOfRangeException
// IMPLEMENTED :  py::class_<IndexOutOfRangeException, MessageBufferException> 
// IMPLEMENTED :    IndexOutOfRangeException_(m,"IndexOutOfRangeException", OPENVDS_DOCSTRING(IndexOutOfRangeException));
// IMPLEMENTED :  IndexOutOfRangeException_.def(py::init<const char *                  >(), py::arg("errorMessage").none(false), OPENVDS_DOCSTRING(IndexOutOfRangeException_IndexOutOfRangeException));
// IMPLEMENTED :  IndexOutOfRangeException_.def(py::init<const native::IndexOutOfRangeException &>(), py::arg("other").none(false), OPENVDS_DOCSTRING(IndexOutOfRangeException_IndexOutOfRangeException_2));
// IMPLEMENTED :  IndexOutOfRangeException_.def("operator_assign"             , static_cast<native::IndexOutOfRangeException &(IndexOutOfRangeException::*)(const native::IndexOutOfRangeException &)>(&IndexOutOfRangeException::operator=), py::arg("other").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(IndexOutOfRangeException_operator_assign));
// IMPLEMENTED :  IndexOutOfRangeException_.def("getErrorMessage"             , static_cast<const char *(IndexOutOfRangeException::*)() const>(&IndexOutOfRangeException::GetErrorMessage), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(IndexOutOfRangeException_GetErrorMessage));
// IMPLEMENTED :  IndexOutOfRangeException_.def_property_readonly("errorMessage", &IndexOutOfRangeException::GetErrorMessage, OPENVDS_DOCSTRING(IndexOutOfRangeException_GetErrorMessage));
register_exception<IndexOutOfRangeException>(m, "IndexOutOfRangeException");

// IMPLEMENTED :  // ReadErrorException
// IMPLEMENTED :  py::class_<ReadErrorException, MessageBufferException> 
// IMPLEMENTED :    ReadErrorException_(m,"ReadErrorException", OPENVDS_DOCSTRING(ReadErrorException));
// IMPLEMENTED :  ReadErrorException_.def(py::init<const char *, int             >(), py::arg("errorMessage").none(false), py::arg("errorCode").none(false), OPENVDS_DOCSTRING(ReadErrorException_ReadErrorException));
// IMPLEMENTED :  ReadErrorException_.def(py::init<const native::ReadErrorException &>(), py::arg("other").none(false), OPENVDS_DOCSTRING(ReadErrorException_ReadErrorException_2));
// IMPLEMENTED :  ReadErrorException_.def("operator_assign"             , static_cast<native::ReadErrorException &(ReadErrorException::*)(const native::ReadErrorException &)>(&ReadErrorException::operator=), py::arg("other").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(ReadErrorException_operator_assign));
// IMPLEMENTED :  ReadErrorException_.def("getErrorMessage"             , static_cast<const char *(ReadErrorException::*)() const>(&ReadErrorException::GetErrorMessage), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(ReadErrorException_GetErrorMessage));
// IMPLEMENTED :  ReadErrorException_.def_property_readonly("errorMessage", &ReadErrorException::GetErrorMessage, OPENVDS_DOCSTRING(ReadErrorException_GetErrorMessage));
// IMPLEMENTED :  ReadErrorException_.def("getErrorCode"                , static_cast<int(ReadErrorException::*)() const>(&ReadErrorException::GetErrorCode), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(ReadErrorException_GetErrorCode));
// IMPLEMENTED :  ReadErrorException_.def_property_readonly("errorCode", &ReadErrorException::GetErrorCode, OPENVDS_DOCSTRING(ReadErrorException_GetErrorCode));
register_exception<ReadErrorException>(m, "ReadErrorException");

py::class_<ReadErrorException>
  ReadErrorException_(m,"ReadErrorException_", OPENVDS_DOCSTRING(ReadErrorException));
ReadErrorException_.def(py::init<const char *, int             >(), py::arg("errorMessage").none(false), py::arg("errorCode").none(false), OPENVDS_DOCSTRING(ReadErrorException_ReadErrorException));
ReadErrorException_.def(py::init<const native::ReadErrorException &>(), py::arg("other").none(false), OPENVDS_DOCSTRING(ReadErrorException_ReadErrorException_2));
ReadErrorException_.def("operator_assign"             , static_cast<native::ReadErrorException &(ReadErrorException::*)(const native::ReadErrorException &)>(&ReadErrorException::operator=), py::arg("other").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(ReadErrorException_operator_assign));
ReadErrorException_.def("getErrorMessage"             , static_cast<const char *(ReadErrorException::*)() const>(&ReadErrorException::GetErrorMessage), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(ReadErrorException_GetErrorMessage));
ReadErrorException_.def_property_readonly("errorMessage", &ReadErrorException::GetErrorMessage, OPENVDS_DOCSTRING(ReadErrorException_GetErrorMessage));
ReadErrorException_.def("getErrorCode"                , static_cast<int(ReadErrorException::*)() const>(&ReadErrorException::GetErrorCode), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(ReadErrorException_GetErrorCode));
ReadErrorException_.def_property_readonly("errorCode", &ReadErrorException::GetErrorCode, OPENVDS_DOCSTRING(ReadErrorException_GetErrorCode));
ReadErrorException_.def("reRaise", [](ReadErrorException &exception) { throw exception; }, py::call_guard<py::gil_scoped_release>());

}

