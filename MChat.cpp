#include "MChat_Core\My_Library\LOG.hpp"

#include "MChat_Core\MChat_Base.cpp"

#include <sstream>
#include <string>

extern bool DEBUG;

using namespace std;

int main () {
  DEBUG = true;
  MChat_Base master = MChat_Base();
  master.start();
}
