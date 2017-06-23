#pragma once

#include <vector>
#include <string>

namespace james {

  struct Material {
    std::string id;
  };

  struct Mesh3d {
    static const unsigned int NOT_PRESENT = (unsigned int)-1;

    unsigned int xyzOffset;
    unsigned int uvOffset;
    unsigned int normalsOffset;
    unsigned int stride;

    std::string id;
    Material* material;
    std::vector<float> data;
  };

  struct Model3d {

    typedef std::vector<Material> MaterialList;
    typedef std::vector<Mesh3d> MeshList;

    Model3d() {}
    Model3d(MaterialList&&, MeshList&&);

  private:
    std::vector<Material> materials_;
    std::vector<Mesh3d> meshes_;
  };

} // namespace james