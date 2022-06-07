#include "SEGYUtils/VDSSEGYInfo.h"

namespace SEGY
{

int32_t
GetVDSIntegerOffsetForDataSampleFormat(SEGY::BinaryHeader::DataSampleFormatCode dataSampleFormatCode)
{
  switch (dataSampleFormatCode)
  {
    // VDS does not support IntegerOffset for Int32 and Int64 so updating this function
    // is not enough to properly handle Int32 types without converting them to float.
  case SEGY::BinaryHeader::DataSampleFormatCode::Int16:
    return 32768;
  case SEGY::BinaryHeader::DataSampleFormatCode::Int8:
    return 128;
  default:
    return 0;
  }
}

OpenVDS::VolumeDataFormat
convertSegyFormat(SEGY::BinaryHeader::DataSampleFormatCode dataSampleFormatCode, OpenVDS::Error& error)
{
  switch (dataSampleFormatCode)
  {
  case SEGY::BinaryHeader::DataSampleFormatCode::IEEEDouble:
  case SEGY::BinaryHeader::DataSampleFormatCode::UInt64:
  case SEGY::BinaryHeader::DataSampleFormatCode::Int64:
    return OpenVDS::VolumeDataFormat::Format_R64;

  case SEGY::BinaryHeader::DataSampleFormatCode::IBMFloat:
  case SEGY::BinaryHeader::DataSampleFormatCode::IEEEFloat:
  case SEGY::BinaryHeader::DataSampleFormatCode::UInt32:
  case SEGY::BinaryHeader::DataSampleFormatCode::Int32:
    return OpenVDS::VolumeDataFormat::Format_R32;

  case SEGY::BinaryHeader::DataSampleFormatCode::UInt16:
  case SEGY::BinaryHeader::DataSampleFormatCode::Int16:
    return OpenVDS::VolumeDataFormat::Format_U16;

  case SEGY::BinaryHeader::DataSampleFormatCode::UInt8:
  case SEGY::BinaryHeader::DataSampleFormatCode::Int8:
    return OpenVDS::VolumeDataFormat::Format_U8;

  case SEGY::BinaryHeader::DataSampleFormatCode::Unknown:
  case SEGY::BinaryHeader::DataSampleFormatCode::UInt24:
  case SEGY::BinaryHeader::DataSampleFormatCode::Int24:
  case SEGY::BinaryHeader::DataSampleFormatCode::FixedPoint:
  default:
    error.code = -1;
    error.string = "Unknown data sample format";
    break;
  }
  return OpenVDS::VolumeDataFormat::Format_Any;
}

} // end namespace SEGY
