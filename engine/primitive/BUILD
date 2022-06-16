package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "primitive",
  srcs = [
    "cube_mesh.h",
    "cube_mesh.cc",
    "sphere_mesh.h",
    "sphere_mesh.cc",
    "plane_mesh.h",
    "plane_mesh.cc",
    "primitive_util.h",
    "primitive_util.cc",
  ],
  deps = [
    "//engine:debug",
    "//engine:engine",
  ]
)