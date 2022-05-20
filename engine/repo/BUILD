package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "repo",
  srcs = [
    "mesh_repo.h",
    "mesh_repo.cc",
    "model_repo.h",
    "model_repo.cc",
    "shader_repo.h",
    "shader_repo.cc",
    "texture_repo.h",
    "texture_repo.cc",
  ],
  deps = [
    "//engine:debug",
    "//engine:engine",
    "//engine/primitive:primitive",
    "//engine:util",
    "@assimp//:assimp",
    "@fmt",
    "@stb",
  ],
)