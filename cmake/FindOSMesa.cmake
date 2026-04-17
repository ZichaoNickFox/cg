if(WIN32)
  set(OSMesa_FOUND FALSE)
  return()
endif()

include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PKG_OSMESA QUIET osmesa)
endif()

set(_cg_osmesa_hints)
foreach(_cg_osmesa_root
        "${CG_OSMESA_ROOT}"
        "$ENV{CG_OSMESA_ROOT}"
        "$ENV{OSMESA_ROOT}")
  if(_cg_osmesa_root)
    list(APPEND _cg_osmesa_hints "${_cg_osmesa_root}")
  endif()
endforeach()

find_path(
  OSMESA_INCLUDE_DIR
  NAMES GL/osmesa.h
  HINTS ${_cg_osmesa_hints} ${PKG_OSMESA_INCLUDE_DIRS}
  PATH_SUFFIXES include include/mesa
)

find_library(
  OSMESA_LIBRARY
  NAMES OSMesa libOSMesa
  HINTS ${_cg_osmesa_hints} ${PKG_OSMESA_LIBRARY_DIRS}
  PATH_SUFFIXES lib lib64
)

set(OSMESA_INCLUDE_DIRS ${OSMESA_INCLUDE_DIR})
set(OSMESA_LIBRARIES ${OSMESA_LIBRARY})

find_package_handle_standard_args(
  OSMesa
  REQUIRED_VARS OSMESA_INCLUDE_DIR OSMESA_LIBRARY
)

if(OSMesa_FOUND AND NOT TARGET OSMesa::OSMesa)
  add_library(OSMesa::OSMesa UNKNOWN IMPORTED)
  set_target_properties(
    OSMesa::OSMesa
    PROPERTIES
      IMPORTED_LOCATION "${OSMESA_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OSMESA_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(OSMESA_INCLUDE_DIR OSMESA_LIBRARY)
