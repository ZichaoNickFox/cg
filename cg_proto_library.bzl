load("@rules_cc//cc:defs.bzl", "cc_proto_library")

def cg_proto_library(
  name,
  srcs,
  deps = []):
  native.proto_library(
    name = name,
    srcs = srcs,
    deps = deps,
  )
  native.cc_proto_library(
    name = "cc_" + name,
    deps = [":" + name]
  )