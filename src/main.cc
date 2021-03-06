#include "application.h"
#include "logging.h"

int main(int argc, char *argv[]) {
  // Initialize Google’s logging library.
  initLogging(argv[0]);

  LOG(INFO) << "Application starting..";

  Application application;
  try {
    application.run();
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << e.what();
    return EXIT_FAILURE;
  }
}
