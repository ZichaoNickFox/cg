
# build
bazel build \
  --cxxopt="--std=c++14" \
  --cxxopt="-Wreturn-type" \
  --cxxopt="-Wnonportable-include-path" \
  --cxxopt="-Wunused-private-field" \
  --cxxopt="-Wdelete-non-abstract-non-virtual-dtor" \
  --cxxopt="-Wunused-variable" \
  --cxxopt="-Wint-to-void-pointer-cast" \
  --cxxopt="-Wreturn-stack-address" \
  --cxxopt="-Wunused-private-field" \
  //playground:launcher

# run
if [ $? -eq '0' ]; then
  ./bazel-bin/playground/launcher
fi