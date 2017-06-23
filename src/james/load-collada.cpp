#include "load-collada.hpp"

#include <james/expat-parser.hpp>
#include <james/expat-facade.hpp>
#include "collada/builder.hpp"

using namespace james::collada;

namespace james {

  Model3d LoadCollada(std::istream& src) {
    
    ExpatFacade facade;
    ExpatParser parser(facade.XMLConsumer());
    Builder builder(facade);

    ParseStream(parser, src);

    return Model3d();
  }

} // namespace james
