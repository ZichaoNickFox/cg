# Before using http_archive, we can use local_repository to test BUILD file
# 
# local_repository(
#   name = "imgui",
#   path = "thirdparty/imgui"
# )

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_proto",
    sha256 = "e017528fd1c91c5a33f15493e3a398181a9e821a804eb7ff5acdd1d2d6c2b18d",
    strip_prefix = "rules_proto-4.0.0-3.20.0",
    urls = [
        "https://github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0-3.20.0.tar.gz",
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
  name = "glm",
  build_file = "@//:thirdparty/glm.BUILD",
  sha256 = "37e2a3d62ea3322e43593c34bae29f57e3e251ea89f4067506c94043769ade4c",
  strip_prefix = "glm",
  urls = ["https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip"],
)

# TODO Only support window platform for now
http_archive(
  name = "glew",
  build_file = "@//:thirdparty/glew.BUILD",
  sha256 = "a9046a913774395a095edcc0b0ac2d81c3aacca61787b39839b941e9be14e0d4",
  strip_prefix = "glew-2.2.0",
  urls = ["https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip"],
)

http_archive(
  name = "glfw",
  build_file = "@//:thirdparty/glfw.BUILD",
  sha256 = "4ef0c544a8ace9a6cd0e0aef8250090f89fea1bf96e9fc1d9d6f76386c290c9c",
  strip_prefix = "glfw-3.3.7",
  urls = ["https://github.com/glfw/glfw/releases/download/3.3.7/glfw-3.3.7.zip"],
)

http_archive(
  name = "assimp",
  build_file = "@//:thirdparty/assimp.BUILD",
  sha256 = "070c9a7f8303d6f3ff3f29a9e047276666a9fade0a97e1123269d22084da3b6c",
  strip_prefix = "cg_assimp-cg_assimp",
  urls = ["https://github.com/ZichaoNickFox/cg_assimp/archive/refs/tags/cg_assimp.zip"],
)

_RULES_BOOST_COMMIT = "652b21e35e4eeed5579e696da0facbe8dba52b1f"

http_archive(
    name = "com_github_nelhage_rules_boost",
    sha256 = "c1b8b2adc3b4201683cf94dda7eef3fc0f4f4c0ea5caa3ed3feffe07e1fb5b15",
    strip_prefix = "rules_boost-%s" % _RULES_BOOST_COMMIT,
    urls = [
        "https://github.com/nelhage/rules_boost/archive/%s.tar.gz" % _RULES_BOOST_COMMIT,
    ],
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

http_archive(
  name = "ordered_map",
  build_file = "@//:thirdparty/ordered_map.BUILD",
  sha256 = "49cd436b8bdacb01d5f4afd7aab0c0d6fa57433dfc29d65f08a5f1ed1e2af26b",
  strip_prefix = "ordered-map-1.0.0",
  urls = ["https://github.com/Tessil/ordered-map/archive/refs/tags/v1.0.0.tar.gz"],
)

http_archive(
  name = "eigen",
  build_file = "@//:thirdparty/eigen.BUILD",
  sha256 = "8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72",
  strip_prefix="eigen-3.4.0",
  urls = [ "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz" ],
)

http_archive(
  name = "implot",
  build_file = "@//:thirdparty/implot.BUILD",
  sha256 = "90a4446a8a08005867be6857d6d1b4f978cd49eb74c7c62a87367b3af15530a5",
  strip_prefix="implot-master",
  urls = [ "https://github.com/epezent/implot/archive/refs/heads/master.zip" ],
)