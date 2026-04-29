#pragma once

#include <set>
#include <string>
#include <tsl/ordered_map.h>
#include <unordered_map>
#include <vector>

#include "renderer/shader.h"

namespace cg {

class ShaderParser {
  struct FileMeta {
    std::vector<std::string> dependances;
    int line_num;
    std::string content;
  };

 public:
  ShaderParser(const std::string& name);
  std::vector<ShaderCodePart> Parse(const std::string& file_path);

 private:
  void ParseAFile(const std::string& file_path);
  std::string ResolvePath(const std::string& requested_path, const std::string& including_file_path) const;
  std::vector<std::string> TopologicalSort();

  tsl::ordered_map<std::string, FileMeta> file_meta_map_;
  std::set<std::string> active_parse_stack_;
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
  ShaderProgramDesc LoadProgramDesc(const std::string& name,
                                    const std::unordered_map<FileType, std::string>& file_paths);
  ShaderProgram Load(const std::string& name, const std::unordered_map<FileType, std::string>& file_paths);
};

};
