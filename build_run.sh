
# build
bazel build \
  --cxxopt="--std=c++14" \
  --cxxopt="-Wreturn-type" \
  --cxxopt="-Wnonportable-include-path" \
  --cxxopt="-Wunused-private-field" \
  --cxxopt="-Wdelete-non-abstract-non-virtual-dtor" \
  --cxxopt="-Wunused-variable" \
  --cxxopt="-Wint-to-void-pointer-cast" \
  //playground:playground

# run
if [ $? -eq '0' ]; then
  ./bazel-bin/playground/playground
fi