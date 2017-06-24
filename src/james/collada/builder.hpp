#pragma once

#include <james/expat-facade.hpp>
#include "lib-geometries-builder.hpp"

namespace james {
namespace collada {

  struct Builder {
    Builder(ExpatFacade&);

  private:
    LibGeometriesBuilder libGeometriesBuilder_;
  };

} // namespace collada
} // namespace james