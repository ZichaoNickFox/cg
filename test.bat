bazel test --test_output=all ^
  --cxxopt="/std:c++20" ^
  --verbose_failures  ^
  //playground/test:eigen_test