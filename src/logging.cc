#include "logging.h"

#include <iomanip>

void logPrefix(std::ostream &s, const google::LogMessageInfo &l, void *) {
  const char *severity = l.severity;
  if (severity[0] == 'I') {
    logInfo(s, l);
  } else if (severity[0] == 'E') {
    logError(s, l);
  } else if (severity[0] == 'W') {
    logWarning(s, l);
  } else if (severity[0] == 'F') {
    logFatal(s, l);
  }
}

void logInfo(std::ostream &s, const google::LogMessageInfo &l) {
  s << '[' << std::setw(2) << l.time.hour() << ':' << std::setw(2)
    << l.time.min() << ':' << std::setw(2) << l.time.sec() << "."
    << std::setw(6) << l.time.usec() << ' ' << std::setfill(' ') << std::setw(5)
    << l.thread_id << std::setfill('0') << " \033[1;32m" << l.severity
    << "\033[0m]";
}

void logError(std::ostream &s, const google::LogMessageInfo &l) {
  s << '[' << std::setw(2) << l.time.hour() << ':' << std::setw(2)
    << l.time.min() << ':' << std::setw(2) << l.time.sec() << "."
    << std::setw(6) << l.time.usec() << ' ' << std::setfill(' ') << std::setw(5)
    << l.thread_id << std::setfill('0') << ' ' << l.filename << ':'
    << l.line_number << " \033[1;31m" << l.severity << "\033[0m]";
}

void logFatal(std::ostream &s, const google::LogMessageInfo &l) {
  s << '[' << std::setw(2) << l.time.hour() << ':' << std::setw(2)
    << l.time.min() << ':' << std::setw(2) << l.time.sec() << "."
    << std::setw(6) << l.time.usec() << ' ' << std::setfill(' ') << std::setw(5)
    << l.thread_id << std::setfill('0') << ' ' << l.filename << ':'
    << l.line_number << " \033[1;30;41m" << l.severity << "\033[0m]";
}

void logWarning(std::ostream &s, const google::LogMessageInfo &l) {
  s << '[' << std::setw(2) << l.time.hour() << ':' << std::setw(2)
    << l.time.min() << ':' << std::setw(2) << l.time.sec() << "."
    << std::setw(6) << l.time.usec() << ' ' << std::setfill(' ') << std::setw(5)
    << l.thread_id << std::setfill('0') << ' ' << l.filename << ':'
    << l.line_number << " \033[1;33m" << l.severity << "\033[0m]";
}