package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "glew",
  srcs = [
    "include/GL/eglew.h",
    "include/GL/glew.h",
    "include/GL/glxew.h",
    "include/GL/wglew.h",
    "src/glew.c",
    "src/glewinfo.c",
    "src/visualinfo.c",
  ],
  includes = ["include"],
  defines = [
    "GLEW_STATIC"
  ],
  linkopts = [
    "-DEFAULTLIB:opengl32.lib",
  ],
)