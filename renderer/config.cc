#include "renderer/config.h"

#include <filesystem>

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

std::string ResolveConfigPath(const std::string& config_dir, const std::string& path) {
  if (path.empty()) {
    return path;
  }
  const std::filesystem::path raw_path(path);
  if (raw_path.is_absolute() || std::filesystem::exists(raw_path)) {
    return util::ReplaceBackslash(raw_path.string());
  }

  const std::filesystem::path project_relative_path = std::filesystem::path(CG_PROJECT_SOURCE_DIR) / raw_path;
  if (std::filesystem::exists(project_relative_path)) {
    return util::ReplaceBackslash(project_relative_path.string());
  }

  const std::filesystem::path config_relative_path = std::filesystem::path(config_dir) / raw_path;
  if (std::filesystem::exists(config_relative_path)) {
    return util::ReplaceBackslash(config_relative_path.string());
  }

  return util::ReplaceBackslash(path);
}
}

void Config::Init(const std::string& config_path) {
  ConfigData config_data;
  std::string content;
  config_dir_ = util::FileDir(config_path);
  util::ReadFileToString(config_path, &content);
  ParseFromString(content, &config_data);
  for (const ShaderConfig& shader_config : config_data.shader_config()) {
    ShaderConfig resolved_shader_config = shader_config;
    if (resolved_shader_config.has_vs_path()) {
      resolved_shader_config.set_vs_path(ResolveConfigPath(config_dir_, resolved_shader_config.vs_path()));
    }
    if (resolved_shader_config.has_fs_path()) {
      resolved_shader_config.set_fs_path(ResolveConfigPath(config_dir_, resolved_shader_config.fs_path()));
    }
    if (resolved_shader_config.has_gs_path()) {
      resolved_shader_config.set_gs_path(ResolveConfigPath(config_dir_, resolved_shader_config.gs_path()));
    }
    if (resolved_shader_config.has_ts_path()) {
      resolved_shader_config.set_ts_path(ResolveConfigPath(config_dir_, resolved_shader_config.ts_path()));
    }
    if (resolved_shader_config.has_cs_path()) {
      resolved_shader_config.set_cs_path(ResolveConfigPath(config_dir_, resolved_shader_config.cs_path()));
    }
    shader_configs_[resolved_shader_config.name()] = resolved_shader_config;
  }
  for (const TextureConfig& texture_config: config_data.texture_config()) {
    TextureConfig resolved_texture_config = texture_config;
    resolved_texture_config.clear_path();
    for (const std::string& path : texture_config.path()) {
      resolved_texture_config.add_path(ResolveConfigPath(config_dir_, path));
    }
    texture_configs_[resolved_texture_config.name()] = resolved_texture_config;
  }
  for (const ModelConfig& model_config : config_data.model_config()) {
    ModelConfig resolved_model_config = model_config;
    resolved_model_config.set_model_dir(ResolveConfigPath(config_dir_, resolved_model_config.model_dir()));
    model_configs_[resolved_model_config.name()] = resolved_model_config;
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

std::string Config::ResolvePath(const std::string& path) const {
  if (path.empty()) {
    return path;
  }
  return ResolveConfigPath(config_dir_, path);
}

} // namespace cg
