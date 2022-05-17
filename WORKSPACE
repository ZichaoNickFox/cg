# Before using http_archive, we can use local_repository to test BUILD file
# 
# local_repository(
#   name = "imgui",
#   path = "thirdparty/imgui"
# )

local_repository(
  name = "glfw",
  path = "thirdparty/glfw"
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
  name = "rules_proto",
  sha256 = "66bfdf8782796239d3875d37e7de19b1d94301e8972b3cbd2446b332429b4df1",
  strip_prefix = "rules_proto-4.0.0",
  urls = [
    "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
    "https://github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
  ],
)
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")
rules_proto_dependencies()
rules_proto_toolchains()

http_archive(
  name = "gtest",
  sha256 = "5cf189eb6847b4f8fc603a3ffff3b0771c08eec7dd4bd961bfd45477dd13eb73",
  strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
  urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
)

http_archive(
  name = "imgui",
  build_file = "@//:thirdparty/imgui.BUILD",
  sha256 = "1770ebb8cee0c408c71a79fa6f386ac2d87aad07843a62a1760350678f0c05c0",
  strip_prefix = "imgui-1.87",
  urls = ["https://github.com/ocornut/imgui/archive/refs/tags/v1.87.zip"],
)

http_archive(
  name = "stb",
  build_file = "@//:thirdparty/stb.BUILD",
  sha256 = "4ce38167f0e4dbf295552df0351dbbb12c0fc3138450ad16a8bfb39c94a0d990",
  strip_prefix = "stb-master",
  urls = ["https://github.com/nothings/stb/archive/refs/heads/master.zip"],
)

http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)

http_archive(
  name = "fmt",
  build_file = "@//:thirdparty/fmt.BUILD",
  sha256 = "23778bad8edba12d76e4075da06db591f3b0e3c6c04928ced4a7282ca3400e5d",
  strip_prefix = "fmt-8.1.1",
  urls = ["https://github.com/fmtlib/fmt/releases/download/8.1.1/fmt-8.1.1.zip"],
)

# http_archive(
#   name = "glfw",
#   build_file = "@//:thirdparty/glfw.BUILD",
#   sha256 = "4ef0c544a8ace9a6cd0e0aef8250090f89fea1bf96e9fc1d9d6f76386c290c9c",
#   strip_prefix = "glfw-3.3.7",
#   urls = ["https://github.com/glfw/glfw/releases/download/3.3.7/glfw-3.3.7.zip"],
# )