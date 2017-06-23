#include "model-3d.hpp"

namespace james {

  Model3d::Model3d(MaterialList&& materials, MeshList&& meshes)
    : materials_(materials), meshes_(meshes)
  {
  }

} // namespace james