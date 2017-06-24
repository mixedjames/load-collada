#include "lib-geometries-builder.hpp"

using namespace std;

namespace james {
namespace collada {

  LibGeometriesBuilder::LibGeometriesBuilder(ExpatFacade& src) {

    ResetAccumulators();

    // Listener 1: /COLLADA/library_geometries/geometry
    //
    // Two jobs:
    // (1) Store the id attribute in <geometry>
    // (2) Save the mesh to this->meshes_ in </geometry>

    src.ListenFor("/COLLADA/library_geometries/geometry", Tag()
      .Opened([this](const Path& p, const Attributes& attr) {
        const char* tmpId = attr["id"];
        if (tmpId) {
          currentMesh_.id = tmpId;
        }
      })
      .Closed([this](const Path& p) {
        // We don't support meshes without IDs or empty meshes.
        // FIXME: ought to report this to caller somehow - it's probably not a fatal
        //        error in most cases so need some warning/logging mechanism
        if (currentMesh_.id.size() > 0 && currentMesh_.parts.size() > 0) {
          meshes_.insert(make_pair(
            currentMesh_.id,
            Mesh(move(currentMesh_.sources), move(currentMesh_.accessors), move(currentMesh_.vertexLink), move(currentMesh_.parts))
          ));
        }
        ResetAccumulators();
      })
    );

    // Listener 2: /COLLADA/library_geometries/geometry/mesh/source/float_array
    //
    // Three tasks:
    // (a) Store the id attribute
    // (b) Accumulate the array text (remembering that .Text may be called more than once if
    //     extra tags appear within the <float_array> tag)
    // (c) Once all the text is accumulated, convert it to an array of floats and store it
    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/source/float_array", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
      const char* tmpId = attr["id"];
      if (tmpId) {
        currentSource_.id = tmpId;
      }
    })
      .Text([this](const Path&, const string& s) {
      // In a valid COLLADA document it is unlikely (?invalid) that <float_array> has
      // any children; however, for robustness (& in case my reading of the spec is wrong)
      // we handle this case by simply accumulating all text until the end tag is found.
      currentSource_.buffer << s;
    })
      .Closed([this](const Path&) {
      if (currentSource_.id.size() > 0) {
        FloatSource src;
        float f;

        while (currentSource_.buffer >> f) {
          src.push_back(f);
        }

        if (src.size() > 0) {
          currentMesh_.sources.insert(make_pair(currentSource_.id, src));
        }
      }
      ResetSourceAccumulator();
    })
      );

    // Listener 3+4+5:
    //    /COLLADA/library_geometries/geometry/mesh/source
    //    /COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor
    //    /COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param
    //
    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/source", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        const char* tmpId = attr["id"];
        if (tmpId) {
          currentAccessor_.id = tmpId;
        }
      })
      .Closed([this](const Path&) {
        if (currentAccessor_.id.size() > 0 && currentAccessor_.nParamsFound > 0) {
          currentMesh_.accessors.insert(make_pair(currentAccessor_.id, currentAccessor_.data));
        }
        ResetAccessorAccumulator();
      })
    );

    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        const char* source = attr["source"];
        const char* count = attr["count"];
        const char* stride = attr["stride"];
        const char* offset = attr["offset"];

        if (source) {
          currentAccessor_.data.source = source;
        }

        if (count) { currentAccessor_.data.count = strtoul(count, nullptr, 0); }
        if (stride) { currentAccessor_.data.stride = strtoul(stride, nullptr, 0); }
        if (offset) { currentAccessor_.data.offset = strtoul(offset, nullptr, 0); }
      })
    );

    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        // Notes re: COLLADA accessors (the spec is quite complicated & several variants are possible)
        // See https://www.khronos.org/collada/wiki/Using_accessors for useful guide.
        //
        // Basic ideas:
        // (a) <param> tags *must* have a type, and *may* have a name
        // (b) Tags without names are spacers and should be skipped
        // (c) Tags with names are valid parameters; however, the names are NOT used and
        //     their semantics are actually determined by their usage later on
        //
        // My algorithm:
        //   - currentIndex counts how many <params> we have seen
        //   - nParamsFound tracks the number of *named* (i.e. valid) params we've seen
        //   - a <param> is considered to be valid if it has a name, a type, and its name
        //     is at least 1 char long
        //   - We only look for the first 3 valid <params> and store them in the (abc)Index
        //     slot.

        const char* name = attr["name"];
        const char* type = attr["type"];

        if (name && type && strlen(name) > 0) {
          // Switch converts nParamsFound (0,1,2) into a, b or c
          switch (currentAccessor_.nParamsFound) {
          case 0:
            currentAccessor_.data.aIndex = currentAccessor_.currentIndex;
            break;
          case 1:
            currentAccessor_.data.bIndex = currentAccessor_.currentIndex;
            break;
          case 2:
            currentAccessor_.data.cIndex = currentAccessor_.currentIndex;
            break;
          }

          currentAccessor_.nParamsFound++;
        }
        currentAccessor_.currentIndex++;
      })
    );

    // Listener 6+7:
    //    /COLLADA/library_geometries/geometry/mesh/vertices
    //    /COLLADA/library_geometries/geometry/mesh/vertices/input
    //
    // I'm not quite sure what the point of the <vertices> tag is; but, it's there so
    // we have to handle it.
    //
    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/vertices", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        const char* id = attr["id"];
        if (id) {
          currentMesh_.vertexLink.id = id;
        }
      })
    );

    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/vertices/input", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        const char* semantic = attr["semantic"];
        const char* source = attr["source"];
        if (semantic && source && strcmp(semantic, "POSITION") == 0) {
          currentMesh_.vertexLink.accessor = source;
        }
      })
    );

    // Listener 8+9+10+11:
    //    /COLLADA/library_geometries/geometry/mesh/polylist
    //    /COLLADA/library_geometries/geometry/mesh/polylist/input
    //    /COLLADA/library_geometries/geometry/mesh/polylist/vcount
    //    /COLLADA/library_geometries/geometry/mesh/polylist/p
    //
    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/polylist", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        const char* material = attr["material"];
        if (material) {
          currentVertexIndex_.data.material = material;
        }
      })
      .Closed([this](const Path&) {
        if (currentVertexIndex_.trianglesOnly && currentVertexIndex_.data.indices.size() > 0
          && currentVertexIndex_.data.position.accessor.size() != 0
        ) {
          currentMesh_.parts.push_back(currentVertexIndex_.data);
        }
        ResetVertexIndexAccumulator();
      })
    );

    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/polylist/input", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        const char* semantic = attr["semantic"];
        const char* source = attr["source"];
        const char* offset = attr["offset"];

        size_t offsetInt = 0;
        if (offset) { offsetInt = strtoul(offset, nullptr, 0); }

        if (semantic && source) {
          if (strcmp(semantic, "VERTEX") == 0) {
            currentVertexIndex_.data.position.accessor = source;
            currentVertexIndex_.data.position.offset = offsetInt;
          }
          else if (strcmp(semantic, "NORMAL") == 0) {
            currentVertexIndex_.data.normals.accessor = source;
            currentVertexIndex_.data.normals.offset = offsetInt;
          }
          else if (strcmp(semantic, "TEXCOORD") == 0) {
            currentVertexIndex_.data.texCoords.accessor = source;
            currentVertexIndex_.data.texCoords.offset = offsetInt;
          }
        }
      })
    );

    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/polylist/vcount", Tag()
      .Opened([this](const Path&, const Attributes&) {
        currentVertexIndex_.vCountBuffer = stringstream();
      })
      .Text([this](const Path&, const string& s) {
        currentVertexIndex_.vCountBuffer << s;
      })
        .Closed([this](const Path&) {
        unsigned int i;

        while (currentVertexIndex_.vCountBuffer >> i) {
          if (i != 3) { currentVertexIndex_.trianglesOnly = false; }
        }
      })
    );

    src.ListenFor("/COLLADA/library_geometries/geometry/mesh/polylist/p", Tag()
      .Opened([this](const Path&, const Attributes&) {
        currentVertexIndex_.pBuffer = stringstream();
      })
      .Text([this](const Path&, const string& s) {
        currentVertexIndex_.pBuffer << s;
      })
      .Closed([this](const Path&) {
        VertexIndex::IndexList::value_type index;

        while (currentVertexIndex_.pBuffer >> index) {
          currentVertexIndex_.data.indices.push_back(index);
        }
      })
    );
  }

  void LibGeometriesBuilder::ResetAccumulators() {
    ResetMeshAccumulator();
    ResetSourceAccumulator();
    ResetAccessorAccumulator();
    ResetVertexIndexAccumulator();
  }

  void LibGeometriesBuilder::ResetMeshAccumulator() {
    currentMesh_.id.clear();
    currentMesh_.sources.clear();
    currentMesh_.accessors.clear();
    currentMesh_.parts.clear();
    currentMesh_.vertexLink = VertexLink();
  }

  void LibGeometriesBuilder::ResetSourceAccumulator() {
    currentSource_.id.clear();
    currentSource_.buffer = stringstream();
  }

  void LibGeometriesBuilder::ResetAccessorAccumulator() {
    currentAccessor_.id.clear();
    currentAccessor_.nParamsFound = 0;
    currentAccessor_.currentIndex = 0;
    currentAccessor_.data = Accessor();
  }

  void LibGeometriesBuilder::ResetVertexIndexAccumulator() {
    currentVertexIndex_.data = VertexIndex();
    currentVertexIndex_.trianglesOnly = true;
  }

} // namespace collada
} // namespace james