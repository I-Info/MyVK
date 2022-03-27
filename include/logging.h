#ifndef MYVK_LOGGING_H
#define MYVK_LOGGING_H

#include <glog/logging.h>

void logPrefix(std::ostream &s, const google::LogMessageInfo &l, void *);

static void logInfo(std::ostream &s, const google::LogMessageInfo &l);

static void logError(std::ostream &s, const google::LogMessageInfo &l);

static void logFatal(std::ostream &s, const google::LogMessageInfo &l);

static void logWarning(std::ostream &s, const google::LogMessageInfo &l);

#endif