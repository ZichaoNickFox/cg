if(NOT BUILD_TESTING)
  return()
endif()

add_subdirectory(base)
add_subdirectory(geometry)
add_subdirectory(physics)
add_subdirectory(rhi)
add_subdirectory(renderer)
