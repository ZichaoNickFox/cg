#pragma once

#include <set>
#include <string>
#include <tsl/ordered_map.h>
#include <unordered_map>
#include <vector>

#include "renderer/shader.h"

namespace renderer {

class ShaderParser {
  struct FileMeta {
    std::vector<std::string> dependances;
    int line_num;
    std::string content;
  };

 public:
  ShaderParser(const std::string& name);
  std::vector<ShaderProgram::CodePart> Parse(const std::string& file_path);

 private:
  void ParseAFile(const std::string& file_path);
  std::vector<std::string> TopologicalSort();

  tsl::ordered_map<std::string, FileMeta> file_meta_map_;
  std::string name_;
};

class ShaderLoader {
 public:
  enum FileType {
    kVS = 0,
    kFS,
    kGS,
    kTS,
    kCS
  };
  ShaderProgram Load(const std::string& name, const std::unordered_map<FileType, std::string>& file_paths);
};

};