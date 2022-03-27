#include "application.h"
#include "logging.h"

int main(int argc, char *argv[]) {
  // Initialize Google’s logging library.
  google::InitGoogleLogging(argv[0], &logPrefix);
  FLAGS_logtostderr = 1;

  LOG(INFO) << "Application starting..";

  Application application;
  try {
    application.run();
  } catch (std::runtime_error e) {
    LOG(FATAL) << e.what();
    return EXIT_FAILURE;
  }
}
