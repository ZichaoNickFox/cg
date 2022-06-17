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

#define CGKILL(info) \
  BT(); \
  LOG(FATAL) << "KILL : " #info " "
  

#define CGLOG(...) LOG(ERROR)

#define CGLOG_IF(verbose, condition) LOG_IF(verbose, condition)

#define CGCHECK_NOTNULL(p) CHECK_NOTNULL(p)

#define CGLOG1() CGLOG(ERROR) << "1111111111";
#define CGLOG2() CGLOG(ERROR) << "2222222222";
#define CGLOG3() CGLOG(ERROR) << "3333333333";
#define CGLOG4() CGLOG(ERROR) << "4444444444";
#define CGLOG5() CGLOG(ERROR) << "5555555555";
#define CGLOG6() CGLOG(ERROR) << "6666666666";
#define CGLOG7() CGLOG(ERROR) << "7777777777";
#define CGLOG8() CGLOG(ERROR) << "8888888888";
#define CGLOG9() CGLOG(ERROR) << "9999999999";

#define CGLOGA() CGLOG(ERROR) << "aaaaaaaaaa";
#define CGLOGB() CGLOG(ERROR) << "bbbbbbbbbb";
#define CGLOGC() CGLOG(ERROR) << "cccccccccc";
#define CGLOGD() CGLOG(ERROR) << "dddddddddd";
#define CGLOGE() CGLOG(ERROR) << "eeeeeeeeee";
#define CGLOGF() CGLOG(ERROR) << "ffffffffff";
#define CGLOGG() CGLOG(ERROR) << "gggggggggg";
#define CGLOGH() CGLOG(ERROR) << "hhhhhhhhhh";
#define CGLOGI() CGLOG(ERROR) << "iiiiiiiiii";

#define DEBUG_SLOW
