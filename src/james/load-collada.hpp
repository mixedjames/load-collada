#pragma once

#include <istream>
#include <james/model-3d.hpp>

namespace james {

  Model3d LoadCollada(std::istream& src);

} // namespace james