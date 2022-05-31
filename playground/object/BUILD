package(default_visibility=["//visibility:public"])
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
  name = "object",
  srcs = [
    "billboard.h",
    "billboard.cc",
    "cube.h",
    "cube.cc",
    "directional_light.h",
    "directional_light.cc",
    "empty_object.h",
    "empty_object.cc",
    "lines.h",
    "lines.cc",
    "model.h",
    "model.cc",
    "object.h",
    "object.cc",
    "plane.h",
    "plane.cc",
    "point_light.h",
    "point_light.cc",
    "skybox.h",
    "skybox.cc",
    "sphere.h",
    "sphere.cc",
  ],
  deps = [
    "//engine:engine",
    "//engine/repo:repo",
    "//playground:context",
    "@fmt",
  ],
)