#include "StringToDouble.h"

#include <sstream>
#include <locale.h>
#include <stdint.h>

#include <fmt/format.h>

#ifdef _WIN32
typedef _locale_t locale_t;
#define strtod_l _strtod_l
#define strtof_l _strtof_l
#define strtoll  _strtoi64
#define strtoull _strtoui64
#endif

namespace OpenVDS
{
static locale_t &
GetCLocale()
{
#ifdef _WIN32
  static locale_t hCLocale = _create_locale(LC_CTYPE, "C");
#else
  static locale_t hCLocale = newlocale(LC_CTYPE_MASK, "C", NULL);
#endif
  return hCLocale;
}

double StringToDouble(const std::string& number, Error& error)
{
  char* endptr;
  double ret = strtod_l(number.c_str(), &endptr, GetCLocale());
  if (endptr != (number.data() + number.size()))
  {
    error.code = -1;
    error.string = fmt::format("Failed to parse number {}.", number);
  }
  return ret;
}
}
