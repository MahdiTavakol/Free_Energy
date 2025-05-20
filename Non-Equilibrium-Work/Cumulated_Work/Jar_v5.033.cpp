#include "Jr_v033.h"
#include <memory>

int main(int argc, char **argv) {
  std::unique_ptr<JarCalculator> jc =
      std::make_unique<JarCalculator>(argc, argv);
  jc->initialize();
  jc->calculate();
  jc->write();
  return 0;
}
