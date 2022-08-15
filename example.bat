bazel --output_user_root="./.cache" build ^
  --cxxopt="/std:c++20" ^
  --verbose_failures  ^
  //thirdparty:example

if errorlevel 1 exit /B 1

.\bazel-bin\thirdparty\implot.exe