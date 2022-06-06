package(default_visibility=["//visibility:public"])
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
  name = "object",
  srcs = [
    "billboard_object.h",
    "billboard_object.cc",
    "cube_object.h",
    "cube_object.cc",
    "directional_light_object.h",
    "directional_light_object.cc",
    "empty_object.h",
    "empty_object.cc",
    "lines_object.h",
    "lines_object.cc",
    "model_object.h",
    "model_object.cc",
    "object.h",
    "object.cc",
    "plane_object.h",
    "plane_object.cc",
    "point_light_object.h",
    "point_light_object.cc",
    "skybox_object.h",
    "skybox_object.cc",
    "sphere_object.h",
    "sphere_object.cc",
  ],
  deps = [
    "//engine:engine",
    "//engine/repo:repo",
    "//playground:context",
    "@fmt",
  ],
)