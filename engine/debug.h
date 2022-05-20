#pragma once

#if defined CG_PLATFORM_MACOS
#include <execinfo.h>
#include <unistd.h>
#include <glog/logging.h>
#elif defined CG_PLATFORM_WINDOWS
#include <boost/stacktrace.hpp>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#endif

// Variable Argument Macro (VA_MACRO) upto 6 arguments
#define NUM_ARGS_(_1, _2, _3, _4, _5, _6, TOTAL, ...) TOTAL
#define NUM_ARGS(...) NUM_ARGS_(__VA_ARGS__, 6, 5, 4, 3, 2, 1)

#define CONCATE_(X, Y) X##Y  // Fixed the double '_' from previous code
#define CONCATE(MACRO, NUMBER) CONCATE_(MACRO, NUMBER)
#define VA_MACRO(MACRO, ...) CONCATE(MACRO, NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

// functions
#if defined CG_PLATFORM_MACOS
#define BT() \
  void *bt_array[10]; \
  size_t bt_size; \
  bt_size = backtrace(bt_array, 10); \
  backtrace_symbols_fd(bt_array, bt_size, STDERR_FILENO);
#elif defined CG_PLATFORM_WINDOWS
#define BT() \
  LOG(ERROR) << boost::stacktrace::stacktrace()
#else
#error Implement BT
#endif

#define CGCHECK(condition)  \
  if (!(condition)) { \
    BT(); \
  } \
  LOG_IF(FATAL, GOOGLE_PREDICT_BRANCH_NOT_TAKEN(!(condition))) \
    << "Check failed: " #condition " "

#define CGLOG(...) LOG(ERROR)

#define CGLOG_IF(verbose, condition) LOG_IF(verbose, condition)

#define CGCHECK_NOTNULL(p) CHECK_NOTNULL(p)