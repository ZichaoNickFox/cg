package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "debug",
  srcs = [
    "debug.h"
  ],
  deps = [
    "@boost//:stacktrace",
    "@com_github_google_glog//:glog"
  ],
  defines = select({
    "@platforms//os:windows": [
      "CG_PLATFORM_WINDOWS",
    ],
    "@platforms//os:macos": [
      "CG_PLATFORM_MACOS"
    ],
    "//conditions:default": [
    ],
  }),
)