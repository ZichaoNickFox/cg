set(CG_THIRDPARTY_DIR "${PROJECT_SOURCE_DIR}/thirdparty")
list(APPEND CMAKE_MODULE_PATH "${CG_THIRDPARTY_DIR}/glfw/CMake/modules")

function(cg_require_thirdparty relative_path)
  if(NOT EXISTS "${CG_THIRDPARTY_DIR}/${relative_path}")
    message(
      FATAL_ERROR
        "Missing third-party dependency at ${CG_THIRDPARTY_DIR}/${relative_path}. "
        "Run `git submodule update --init --recursive` before configuring the project."
    )
  endif()
endfunction()

cg_require_thirdparty(googletest/CMakeLists.txt)
cg_require_thirdparty(glog/CMakeLists.txt)
cg_require_thirdparty(glfw/CMakeLists.txt)
cg_require_thirdparty(protobuf/CMakeLists.txt)
cg_require_thirdparty(fmt/CMakeLists.txt)
cg_require_thirdparty(assimp/CMakeLists.txt)
cg_require_thirdparty(glm/glm)
cg_require_thirdparty(generated/glad/src/gl.c)
cg_require_thirdparty(generated/glad/include/glad/gl.h)
cg_require_thirdparty(imgui/imgui.cpp)
cg_require_thirdparty(implot/implot.cpp)
cg_require_thirdparty(ordered-map/include)
cg_require_thirdparty(eigen/Eigen)
cg_require_thirdparty(stb/stb_image.h)

set(CG_BUILD_TESTING ${BUILD_TESTING})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
if(CG_ENABLE_OSMESA)
  find_package(OSMesa QUIET)
  find_package(MesaEGL QUIET)
else()
  set(OSMesa_FOUND FALSE)
  set(MesaEGL_FOUND FALSE)
endif()
if(OSMesa_FOUND)
  set(CG_HAS_OSMESA ON CACHE INTERNAL "")
  set(GLFW_USE_OSMESA ON CACHE BOOL "" FORCE)
else()
  set(CG_HAS_OSMESA OFF CACHE INTERNAL "")
  set(GLFW_USE_OSMESA OFF CACHE BOOL "" FORCE)
endif()
if(MesaEGL_FOUND)
  set(CG_HAS_MESA_EGL ON CACHE INTERNAL "")
  set(CG_MESA_EGL_LIBRARY_DIRS "${MesaEGL_LIBRARY_DIRS}" CACHE INTERNAL "")
else()
  set(CG_HAS_MESA_EGL OFF CACHE INTERNAL "")
  set(CG_MESA_EGL_LIBRARY_DIRS "" CACHE INTERNAL "")
endif()
if(OSMesa_FOUND OR MesaEGL_FOUND)
  set(CG_HAS_MESA_OFFSCREEN ON CACHE INTERNAL "")
else()
  set(CG_HAS_MESA_OFFSCREEN OFF CACHE INTERNAL "")
endif()
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(WITH_GFLAGS OFF CACHE BOOL "" FORCE)
set(WITH_GTEST OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)

add_subdirectory(
  ${CG_THIRDPARTY_DIR}/googletest
  ${CMAKE_BINARY_DIR}/thirdparty/googletest
  EXCLUDE_FROM_ALL
)
add_subdirectory(
  ${CG_THIRDPARTY_DIR}/glog
  ${CMAKE_BINARY_DIR}/thirdparty/glog
  EXCLUDE_FROM_ALL
)
add_subdirectory(
  ${CG_THIRDPARTY_DIR}/glfw
  ${CMAKE_BINARY_DIR}/thirdparty/glfw
  EXCLUDE_FROM_ALL
)
add_subdirectory(
  ${CG_THIRDPARTY_DIR}/protobuf
  ${CMAKE_BINARY_DIR}/thirdparty/protobuf
  EXCLUDE_FROM_ALL
)
add_subdirectory(
  ${CG_THIRDPARTY_DIR}/fmt
  ${CMAKE_BINARY_DIR}/thirdparty/fmt
  EXCLUDE_FROM_ALL
)
add_subdirectory(
  ${CG_THIRDPARTY_DIR}/assimp
  ${CMAKE_BINARY_DIR}/thirdparty/assimp
  EXCLUDE_FROM_ALL
)
set(BUILD_TESTING ${CG_BUILD_TESTING} CACHE BOOL "" FORCE)

if(TARGET glog AND NOT TARGET glog::glog)
  add_library(glog::glog ALIAS glog)
endif()

if(TARGET gtest AND NOT TARGET GTest::gtest)
  add_library(GTest::gtest ALIAS gtest)
endif()

if(TARGET gtest_main AND NOT TARGET GTest::gtest_main)
  add_library(GTest::gtest_main ALIAS gtest_main)
endif()

if(TARGET libprotobuf AND NOT TARGET protobuf::libprotobuf)
  add_library(protobuf::libprotobuf ALIAS libprotobuf)
endif()

if(TARGET protoc AND NOT TARGET protobuf::protoc)
  add_executable(protobuf::protoc ALIAS protoc)
endif()

add_library(cg_glm INTERFACE)
target_include_directories(cg_glm SYSTEM INTERFACE ${CG_THIRDPARTY_DIR}/glm)
add_library(cg::glm ALIAS cg_glm)

add_library(cg_eigen INTERFACE)
target_include_directories(cg_eigen SYSTEM INTERFACE ${CG_THIRDPARTY_DIR}/eigen)
target_compile_definitions(cg_eigen INTERFACE EIGEN_MPL2_ONLY EIGEN_NO_DEBUG)
add_library(cg::eigen ALIAS cg_eigen)

add_library(cg_ordered_map INTERFACE)
target_include_directories(cg_ordered_map SYSTEM INTERFACE ${CG_THIRDPARTY_DIR}/ordered-map/include)
add_library(cg::ordered_map ALIAS cg_ordered_map)

add_library(cg_stb INTERFACE)
target_include_directories(cg_stb SYSTEM INTERFACE ${CG_THIRDPARTY_DIR}/stb)
add_library(cg::stb ALIAS cg_stb)

add_library(
  cg_glad
  STATIC
    ${CG_THIRDPARTY_DIR}/generated/glad/src/gl.c
    ${CG_THIRDPARTY_DIR}/generated/glad/include/glad/gl.h
    ${CG_THIRDPARTY_DIR}/generated/glad/include/KHR/khrplatform.h
)
target_include_directories(cg_glad SYSTEM PUBLIC ${CG_THIRDPARTY_DIR}/generated/glad/include)
target_link_libraries(cg_glad PUBLIC ${CG_OPENGL_TARGET})
add_library(cg::glad ALIAS cg_glad)

add_library(
  cg_imgui
  STATIC
    ${CG_THIRDPARTY_DIR}/imgui/imgui.cpp
    ${CG_THIRDPARTY_DIR}/imgui/imgui_demo.cpp
    ${CG_THIRDPARTY_DIR}/imgui/imgui_draw.cpp
    ${CG_THIRDPARTY_DIR}/imgui/imgui_tables.cpp
    ${CG_THIRDPARTY_DIR}/imgui/imgui_widgets.cpp
    ${CG_THIRDPARTY_DIR}/imgui/imgui.h
    ${CG_THIRDPARTY_DIR}/imgui/imgui_internal.h
    ${CG_THIRDPARTY_DIR}/imgui/imconfig.h
    ${CG_THIRDPARTY_DIR}/imgui/imstb_rectpack.h
    ${CG_THIRDPARTY_DIR}/imgui/imstb_textedit.h
    ${CG_THIRDPARTY_DIR}/imgui/imstb_truetype.h
    ${CG_THIRDPARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${CG_THIRDPARTY_DIR}/imgui/backends/imgui_impl_glfw.h
    ${CG_THIRDPARTY_DIR}/generated/imgui_impl_opengl3_glad.cc
    ${CG_THIRDPARTY_DIR}/imgui/backends/imgui_impl_opengl3.h
    ${CG_THIRDPARTY_DIR}/imgui/backends/imgui_impl_opengl3_loader.h
)
target_include_directories(
  cg_imgui
  SYSTEM PUBLIC
    ${CG_THIRDPARTY_DIR}/imgui
    ${CG_THIRDPARTY_DIR}/imgui/backends
)
target_link_libraries(cg_imgui PUBLIC cg::glad glfw ${CG_OPENGL_TARGET})
add_library(cg::imgui ALIAS cg_imgui)

add_library(
  cg_implot
  STATIC
    ${CG_THIRDPARTY_DIR}/implot/implot.cpp
    ${CG_THIRDPARTY_DIR}/implot/implot_demo.cpp
    ${CG_THIRDPARTY_DIR}/implot/implot_items.cpp
    ${CG_THIRDPARTY_DIR}/implot/implot.h
    ${CG_THIRDPARTY_DIR}/implot/implot_internal.h
)
target_include_directories(cg_implot SYSTEM PUBLIC ${CG_THIRDPARTY_DIR}/implot)
target_link_libraries(cg_implot PUBLIC cg::imgui)
add_library(cg::implot ALIAS cg_implot)
