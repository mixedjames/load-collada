#pragma once

#include <string>
#include <vector>
#include <map>

namespace james {
namespace collada {

  using std::string;
  using std::vector;
  using std::map;

  typedef vector<float> FloatSource;

  struct Accessor {
    static const size_t NOT_PRESENT = (size_t)-1;

    string source;

    size_t count;
    size_t stride;
    size_t offset;

    size_t aIndex;
    size_t bIndex;
    size_t cIndex;

    Accessor()
      : count(0), stride(0), offset(0), aIndex(NOT_PRESENT), bIndex(NOT_PRESENT), cIndex(NOT_PRESENT)
    {}
  };

  struct VertexLink {
    string id;
    string accessor;
  };

  struct VertexIndex {
    typedef vector<unsigned int> IndexList;

    struct Input {
      string accessor;
      size_t offset;

      Input() : offset(0) {}
    };

    string material;
    Input position;
    Input normals;
    Input texCoords;
    IndexList indices;
  };

  struct Mesh {
    typedef map<string, FloatSource> SourceMap;
    typedef map<string, Accessor> AccessorMap;
    typedef vector<VertexIndex> VertexIndexList;

    Mesh() {}
    Mesh(SourceMap&& sources, AccessorMap&& accessors, VertexLink&& vertices, VertexIndexList&& parts)
      : sources_(sources), accessors_(accessors), vertices_(vertices), parts_(parts)
    {}
    Mesh(const SourceMap& sources, const AccessorMap& accessors, const VertexLink& vertices, const VertexIndexList& parts)
      : sources_(sources), accessors_(accessors), vertices_(vertices), parts_(parts)
    {}

    const SourceMap& Sources() const { return sources_; }
    const AccessorMap& Accessors() const { return accessors_; }
    const VertexLink& Vertices() const { return vertices_; }
    const VertexIndexList& Parts() const { return parts_; }

  private:
    SourceMap sources_;
    AccessorMap accessors_;
    VertexLink vertices_;
    VertexIndexList parts_;
  };

} // namespace collada
} // namespace james