
# build
bazel build \
  --cxxopt="--std=c++14" \
  --cxxopt="-Wreturn-type" \
  --cxxopt="-Wnonportable-include-path" \
  --cxxopt="-Wunused-private-field" \
  --cxxopt="-Wdelete-non-abstract-non-virtual-dtor" \
  //playground:playground

# run
if [ $? -eq '0' ]; then
  ./bazel-bin/playground/playground
fi