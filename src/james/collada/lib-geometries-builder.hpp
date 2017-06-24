#pragma once

#include <james/expat-facade.hpp>
#include "dom.hpp"
#include <sstream>

namespace james {
namespace collada {

  using std::stringstream;

  struct LibGeometriesBuilder {
    typedef map<string, Mesh> MeshMap;

    LibGeometriesBuilder(ExpatFacade&);

    const MeshMap& Meshes() const { return meshes_; }

  private:
    struct {
      string id;
      Mesh::SourceMap sources;
      Mesh::AccessorMap accessors;
      VertexLink vertexLink;
      Mesh::VertexIndexList parts;
    } currentMesh_;

    struct {
      string id;
      stringstream buffer;
    } currentSource_;

    struct {
      string id;
      size_t nParamsFound;
      size_t currentIndex;
      Accessor data;
    } currentAccessor_;

    struct {
      bool trianglesOnly;
      VertexIndex data;
      stringstream vCountBuffer;
      stringstream pBuffer;
    } currentVertexIndex_;

    MeshMap meshes_;

    void ResetAccumulators();
    void ResetMeshAccumulator();
    void ResetSourceAccumulator();
    void ResetAccessorAccumulator();
    void ResetVertexIndexAccumulator();
  };

} // namespace collada
} // namespace james