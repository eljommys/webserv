#pragma once

#include <iostream>
#include <sstream>

namespace utils
{
  template< typename T >
  std::string toString(T input)
  {
    std::ostringstream  ss;

    ss << input;
    return (ss.str());
  }

  std::string getFileExtension(std::string const & input);
}