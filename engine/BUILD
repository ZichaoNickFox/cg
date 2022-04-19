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
    "frame_buffer_attachment.h",
    "g_buffer.h",
    "g_buffer.cc",
    "material.h",
    "material.cc",
    "ms_frame_buffer.h",
    "ms_frame_buffer.cc",
    "mesh.h",
    "mesh.cc",
    "pass.h",
    "pass.cc",
    "shader.h",
    "shader.cc",
    "texture.h",
    "texture.cc",
    "transform.h",
    "transform.cc",
  ],
  deps = [
    "//engine:debug",
    "@imgui//:imgui",
  ],
  linkopts = [
    "-lfmt",
    "-lGLEW",
  ],
)