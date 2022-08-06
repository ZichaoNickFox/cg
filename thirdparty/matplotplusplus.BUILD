package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "matplotplusplus",
  srcs = glob([
      "source/matplot/**/*.h",
      "source/matplot/**/*.cpp",
      "source/3rd_party/**/*.h",
      "source/3rd_party/**/*.hpp",
      "source/3rd_party/**/*.cpp",
    ],
    exclude = [
      "source/matplot/backend/*.h",
      "source/matplot/backend/*.cpp",
    ],
  ),
  includes = [
    "source",
    "source/3rd_party/cimg",
    "source/3rd_party/nodesoup/include"
  ],
  defines = [],
)