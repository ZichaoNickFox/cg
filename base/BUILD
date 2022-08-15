package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "debug",
  srcs = [
    "debug.h",
  ],
  deps = [
    "@boost//:stacktrace",
    "@com_github_google_glog//:glog",
    "@glm"
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

cc_library(
  name = "base",
  srcs = [
    "color.h",
    "geometry.h",
    "geometry.cc",
    "math.h",
    "math.cc",
    "util.h",
    "util.cc"
  ],
  deps = [
    "//base:debug",
    "@eigen",
    "@glm"
  ]
)