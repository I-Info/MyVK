#include "application.h"

#include <cstring>
#include <glog/logging.h>
#include <iomanip>
#include <iostream>

void logPrefix(std::ostream &s, const LogMessageInfo &l, void *) {
  s << "[" << l.severity << ' ' << std::setw(2) << l.time.hour() << ':'
    << std::setw(2) << l.time.min() << ':' << std::setw(2) << l.time.sec()
    << "." << std::setw(6) << l.time.usec() << ' ' << std::setfill(' ')
    << std::setw(5) << l.thread_id << std::setfill('0') << ' ' << l.filename
    << ':' << l.line_number << "]";
}

int main(int argc, char *argv[]) {
  // Initialize Google’s logging library.
  google::InitGoogleLogging(argv[0], &logPrefix);
  FLAGS_logtostderr = 1;
  LOG(INFO) << "Application starting..";

  LOG(ERROR) << "error!";
  Application application;
  try {
    application.run();
  } catch (std::runtime_error e) {
    std::cerr << "\033[0;32;31m[error]\033[m " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
