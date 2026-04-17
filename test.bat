@echo off
setlocal

if defined HTTP_PROXY set "http_proxy=%HTTP_PROXY%"
if defined HTTPS_PROXY set "https_proxy=%HTTPS_PROXY%"
if defined NO_PROXY set "no_proxy=%NO_PROXY%"

cmake --preset dev
if errorlevel 1 exit /B 1

cmake --build --preset dev
if errorlevel 1 exit /B 1

ctest --preset dev
