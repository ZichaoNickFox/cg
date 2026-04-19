cmake_minimum_required(VERSION 3.24)

project(cg LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(CG_ENABLE_VULKAN "Build the Vulkan RHI backend when a Vulkan SDK is available." ON)
option(CG_ENABLE_OSMESA "Enable Mesa/llvmpipe offscreen rendering when a compatible Mesa runtime is available." ON)
set(CG_OSMESA_ROOT "" CACHE PATH "Optional root directory of a Mesa install used for llvmpipe (OSMesa or EGL).")

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

function(cg_normalize_proxy_env upper lower)
  if(DEFINED ENV{${upper}} AND NOT "$ENV{${upper}}" STREQUAL "")
    set(ENV{${lower}} "$ENV{${upper}}")
  elseif(DEFINED ENV{${lower}} AND NOT "$ENV{${lower}}" STREQUAL "")
    set(ENV{${upper}} "$ENV{${lower}}")
  endif()
endfunction()

cg_normalize_proxy_env(HTTP_PROXY http_proxy)
cg_normalize_proxy_env(HTTPS_PROXY https_proxy)
cg_normalize_proxy_env(NO_PROXY no_proxy)

find_package(OpenGL REQUIRED)

if(TARGET OpenGL::GL)
  set(CG_OPENGL_TARGET OpenGL::GL)
elseif(TARGET OpenGL::OpenGL)
  set(CG_OPENGL_TARGET OpenGL::OpenGL)
else()
  message(FATAL_ERROR "Unable to find a usable OpenGL CMake target.")
endif()

set(CG_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated CACHE INTERNAL "")
set(CG_OPENGL_TARGET ${CG_OPENGL_TARGET} CACHE INTERNAL "")

include(CTest)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake)

add_library(cg_project_options INTERFACE)
target_compile_features(cg_project_options INTERFACE cxx_std_20)
target_include_directories(
  cg_project_options
  INTERFACE
    ${PROJECT_SOURCE_DIR}
    ${CG_GENERATED_DIR}
)
target_compile_definitions(
  cg_project_options
  INTERFACE
    CG_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    $<$<PLATFORM_ID:Darwin>:CG_PLATFORM_MACOS>
    $<$<PLATFORM_ID:Windows>:CG_PLATFORM_WINDOWS>
)

if(MSVC)
  target_compile_options(cg_project_options INTERFACE /bigobj)
endif()

add_library(cg::project_options ALIAS cg_project_options)

add_subdirectory(base)
add_subdirectory(geometry)
add_subdirectory(physics)
add_subdirectory(renderer/proto)
add_subdirectory(rhi)
add_subdirectory(renderer)
add_subdirectory(playground/geometry_scene)
add_subdirectory(playground/physics_scene)
add_subdirectory(playground/renderer_scene)
add_subdirectory(playground)
add_subdirectory(test)
add_subdirectory(playground/test)
