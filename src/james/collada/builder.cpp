#include "builder.hpp"

#include "exceptions.hpp"

namespace james {
namespace collada {

  namespace {

    void CheckMaxVersion(int maxMaj, int maxMin, const std::string& version) {
      std::size_t firstDot = version.find_first_of('.');
      std::size_t secondDot = version.find_first_of('.', (firstDot != std::string::npos) ? firstDot + 1 : std::string::npos);

      if (version.size() >= 5 && firstDot != std::string::npos && secondDot != std::string::npos) {
        int maj = atoi(version.substr(0, firstDot).c_str());
        int min = atoi(version.substr(firstDot + 1, secondDot - firstDot).c_str());

        if (maj > maxMaj || (maj <= maxMaj && min > maxMin)) {
          throw ColladaIOException("Unsupported COLLADA version.");
        }
      }
      else {
        throw ColladaIOException("Invalid COLLADA version.");
      }
    }


  }

  Builder::Builder(ExpatFacade& src)
    : libGeometriesBuilder_(src)
  {

    src.ListenFor("/COLLADA", Tag()
      .Opened([this](const Path&, const Attributes& attr) {
        CheckMaxVersion(1, 4, attr["version"]);
      })
      .Closed([this](const Path& p) {
      })
    );

  }

} // namespace collada
} // namespace james