bazel test --test_output=all ^
  --cxxopt="/std:c++17" ^
  --verbose_failures  ^
  //playground:test