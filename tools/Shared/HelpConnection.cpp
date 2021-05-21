#include <stdio.h>

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(help);

std::string GetConnectionHelpString()
{
  auto helpfs = cmrc::help::get_filesystem();
  auto connection = helpfs.open("connection.rst");
  std::string connectionstr(connection.begin(), connection.end());
  return connectionstr;
}
