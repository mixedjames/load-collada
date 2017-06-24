#pragma once

#include <stdexcept>

namespace james {

  struct ColladaIOException
    : std::runtime_error
  {
    ColladaIOException(const std::string& msg)
      : runtime_error(msg)
    {}
  };

}