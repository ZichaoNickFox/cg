set(CG_CONFIG_PROTO ${CMAKE_CURRENT_SOURCE_DIR}/config.proto)
set(CG_CONFIG_PROTO_OUT_DIR ${CG_GENERATED_DIR}/renderer/proto)
set(CG_CONFIG_PROTO_CC ${CG_CONFIG_PROTO_OUT_DIR}/config.pb.cc)
set(CG_CONFIG_PROTO_H ${CG_CONFIG_PROTO_OUT_DIR}/config.pb.h)

file(MAKE_DIRECTORY ${CG_CONFIG_PROTO_OUT_DIR})

add_custom_command(
  OUTPUT
    ${CG_CONFIG_PROTO_CC}
    ${CG_CONFIG_PROTO_H}
  COMMAND
    $<TARGET_FILE:protobuf::protoc>
    --proto_path=${PROJECT_SOURCE_DIR}
    --cpp_out=${CG_GENERATED_DIR}
    ${CG_CONFIG_PROTO}
  DEPENDS
    ${CG_CONFIG_PROTO}
    protobuf::protoc
  VERBATIM
)

add_library(
  cg_proto
  STATIC
    ${CG_CONFIG_PROTO_CC}
    ${CG_CONFIG_PROTO_H}
)
target_include_directories(cg_proto PUBLIC ${CG_GENERATED_DIR})
target_link_libraries(cg_proto PUBLIC cg::project_options protobuf::libprotobuf)
add_library(cg::proto ALIAS cg_proto)
