bazel --output_user_root="./.cache" build ^
  --cxxopt="/std:c++17" ^
  --verbose_failures  ^
  //playground:window

if errorlevel 1 exit /B 1

.\bazel-bin\playground\window.exe

