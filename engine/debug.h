#pragma once

#include <execinfo.h>
#include <unistd.h>

#include <glog/logging.h>

#define BT() \
  void *array[10]; \
  size_t size; \
  size = backtrace(array, 10); \
  backtrace_symbols_fd(array, size, STDERR_FILENO);

#define BTCHECK(condition)  \
  if (!(condition)) { \
    BT(); \
  } \
  LOG_IF(FATAL, GOOGLE_PREDICT_BRANCH_NOT_TAKEN(!(condition))) \
    << "Check failed: " #condition " "