#include <iostream>

#include "application.h"

int main() {
  Application application;
  try {
    application.run();
  } catch (std::runtime_error e) {
    std::cerr << "[error]\t" << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
