#include <iostream>
#include <fstream>
#include <james/load-collada.hpp>

using namespace std;
using namespace james;

int main() {
  ifstream src("files/cube.dae");
  src.exceptions(ios::badbit);

  Model3d m(LoadCollada(src));

  cin.get();
}
