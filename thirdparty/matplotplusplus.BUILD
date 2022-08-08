package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "matplotplusplus",
  srcs = glob([
      "source/3rd_party/**/*.h",
      "source/3rd_party/**/*.hpp",
      "source/3rd_party/**/*.cpp",
      "source/matplot/axes_objects/*.h",
      "source/matplot/axes_objects/*.cpp",
      "source/matplot/backend/backend_interface.h",
      "source/matplot/backend/backend_interface.cpp",
      "source/matplot/backend/backend_registry.h",
      "source/matplot/backend/backend_registry.cpp",
      "source/matplot/backend/gnuplot.h",
      "source/matplot/backend/gnuplot.cpp",
      "source/matplot/backend/opengl_embed.h",
      "source/matplot/backend/opengl_embed.cpp",
      "source/matplot/core/*.h",
      "source/matplot/core/*.cpp",
      "source/matplot/freestanding/*.h",
      "source/matplot/freestanding/*.cpp",
      "source/matplot/util/*.h",
      "source/matplot/util/*.cpp",
    ],
  ),
  includes = [
    "source",
    "source/3rd_party/cimg",
    "source/3rd_party/nodesoup/include"
  ],
  defines = [],
  deps = [
    "@glew",
    "@glfw",
  ]
)