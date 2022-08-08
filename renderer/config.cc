#include "renderer/config.h"

#include "base/debug.h"
#include "base/util.h"
#include <google/protobuf/text_format.h>

namespace cg {
namespace {

// proto
template<typename MessageType>
void ParseFromString(const std::string& content, MessageType* message) {
  CGCHECK(google::protobuf::TextFormat::ParseFromString(content, message));
}
template<typename ElemType>
std::vector<ElemType> ProtoRepeatedToVector(const google::protobuf::RepeatedPtrField<ElemType>& data) {
  return {data.begin(), data.end()};
}
template<typename KeyType, typename ValueType>
std::unordered_map<KeyType, ValueType> ProtoMap2UnorderedMap(const google::protobuf::Map<KeyType, ValueType>& map) {
  std::unordered_map<KeyType, ValueType> res;
  for (auto& p : map) {
    res[p.first] = p.second;
  }
  return res;
}
}

void Config::Init(const std::string& config_path) {
  ConfigData config_data;
  std::string content;
  util::ReadFileToString(config_path, &content);
  ParseFromString(content, &config_data);
  for (const ShaderConfig& shader_config : config_data.shader_config()) {
    shader_configs_[shader_config.name()] = shader_config;
  }
  for (const TextureConfig& texture_config: config_data.texture_config()) {
    texture_configs_[texture_config.name()] = texture_config;
  }
  for (const ModelConfig& model_config : config_data.model_config()) {
    model_configs_[model_config.name()] = model_config;
  }
}

const ShaderConfig& Config::shader_config(const std::string& name) const {
  CGCHECK(shader_configs_.find(name) != shader_configs_.end()) << name;
  return shader_configs_.at(name);
}

const TextureConfig& Config::texture_config(const std::string& name) const {
  CGCHECK(texture_configs_.find(name) != texture_configs_.end()) << name;
  return texture_configs_.at(name);
}

const ModelConfig& Config::model_config(const std::string& name) const {
  CGCHECK(model_configs_.find(name) != model_configs_.end()) << "model not in config : model~" << name;
  return model_configs_.at(name);
}

} // namespace cg