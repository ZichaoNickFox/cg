if(WIN32)
  set(MesaEGL_FOUND FALSE)
  return()
endif()

include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PKG_EGL QUIET egl)
  pkg_check_modules(PKG_GL QUIET gl)
endif()

set(_cg_mesa_hints)
foreach(_cg_mesa_root
        "${CG_OSMESA_ROOT}"
        "$ENV{CG_OSMESA_ROOT}"
        "$ENV{OSMESA_ROOT}"
        "${CG_MESA_ROOT}"
        "$ENV{CG_MESA_ROOT}"
        "$ENV{MESA_ROOT}"
        "/opt/homebrew/opt/mesa"
        "/usr/local/opt/mesa")
  if(_cg_mesa_root)
    list(APPEND _cg_mesa_hints "${_cg_mesa_root}")
  endif()
endforeach()

file(GLOB _cg_mesa_cellar_hints
     LIST_DIRECTORIES true
     "/opt/homebrew/Cellar/mesa/*"
     "/usr/local/Cellar/mesa/*")
list(APPEND _cg_mesa_hints ${_cg_mesa_cellar_hints})
list(REMOVE_DUPLICATES _cg_mesa_hints)

find_path(
  MesaEGL_INCLUDE_DIR
  NAMES EGL/egl.h
  HINTS ${_cg_mesa_hints} ${PKG_EGL_INCLUDE_DIRS}
  PATH_SUFFIXES include
)

find_path(
  MesaGL_INCLUDE_DIR
  NAMES GL/glcorearb.h
  HINTS ${_cg_mesa_hints} ${PKG_GL_INCLUDE_DIRS}
  PATH_SUFFIXES include
)

find_library(
  MesaEGL_LIBRARY
  NAMES EGL libEGL
  HINTS ${_cg_mesa_hints} ${PKG_EGL_LIBRARY_DIRS}
  PATH_SUFFIXES lib lib64
)

find_library(
  MesaGL_LIBRARY
  NAMES GL libGL
  HINTS ${_cg_mesa_hints} ${PKG_GL_LIBRARY_DIRS}
  PATH_SUFFIXES lib lib64
)

set(MesaEGL_INCLUDE_DIRS ${MesaEGL_INCLUDE_DIR} ${MesaGL_INCLUDE_DIR})
list(REMOVE_DUPLICATES MesaEGL_INCLUDE_DIRS)

set(MesaEGL_LIBRARIES ${MesaEGL_LIBRARY} ${MesaGL_LIBRARY})

set(MesaEGL_LIBRARY_DIRS)
foreach(_cg_mesa_library ${MesaEGL_LIBRARY} ${MesaGL_LIBRARY})
  if(_cg_mesa_library)
    get_filename_component(_cg_mesa_library_dir "${_cg_mesa_library}" DIRECTORY)
    list(APPEND MesaEGL_LIBRARY_DIRS "${_cg_mesa_library_dir}")
  endif()
endforeach()
list(REMOVE_DUPLICATES MesaEGL_LIBRARY_DIRS)

find_package_handle_standard_args(
  MesaEGL
  REQUIRED_VARS MesaEGL_INCLUDE_DIR MesaGL_INCLUDE_DIR MesaEGL_LIBRARY MesaGL_LIBRARY
)

if(MesaEGL_FOUND AND NOT TARGET MesaEGL::MesaEGL)
  add_library(MesaEGL::MesaEGL INTERFACE IMPORTED)
  set_target_properties(
    MesaEGL::MesaEGL
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${MesaEGL_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${MesaEGL_LIBRARIES}"
  )
endif()

mark_as_advanced(
  MesaEGL_INCLUDE_DIR
  MesaGL_INCLUDE_DIR
  MesaEGL_LIBRARY
  MesaGL_LIBRARY
)
