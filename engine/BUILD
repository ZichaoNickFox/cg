package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "debug",
  srcs = [
    "debug.h"
  ]
)

cc_library(
  name = "engine",
  srcs = [
    "camera.h",
    "camera.cc",
    "color_frame_buffer.h",
    "color_frame_buffer.cc",
    "depth_frame_buffer.h",
    "depth_frame_buffer.cc",
    "frame_buffer.h",
    "frame_buffer.cc",
    "material.h",
    "material.cc",
    "mesh.h",
    "mesh.cc",
    "shader.h",
    "shader.cc",
    "texture.h",
    "texture.cc",
    "transform.h",
    "transform.cc",
  ],
  deps = [
    "//engine:debug"
  ],
  linkopts = [
    "-lfmt",
    "-lGLEW",
  ],
)