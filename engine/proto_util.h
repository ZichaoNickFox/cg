#pragma once

#include <google/protobuf/text_format.h>
#include <glog/logging.h>

namespace engine {
namespace proto_util {
template<typename MessageType>
void ParseFromString(const std::string& content, MessageType* message) {
  CHECK(google::protobuf::TextFormat::ParseFromString(content, message));
}
}
}