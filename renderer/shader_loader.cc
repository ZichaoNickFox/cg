#include "renderer/shader_loader.h"

#include "base/util.h"

namespace renderer {

ShaderParser::ShaderParser(const std::string& name) {
  name_ = name;
}

std::vector<ShaderProgram::CodePart> ShaderParser::Parse(const std::string& file_path) {
  ParseAFile(file_path);

  std::vector<std::string> sorted = TopologicalSort();
  std::vector<ShaderProgram::CodePart> res;
  for (const std::string& path : sorted) {
    res.push_back({path, file_meta_map_[path].content});
  }
  return res;
}

void ShaderParser::ParseAFile(const std::string& file_path) {
  if (file_meta_map_.find(file_path) != file_meta_map_.end()) {
    return;
  }
  const std::string kComment = "\/\/";
  const std::string kIncludeIdentifierPrefix = "#include \"";
  const std::string kIncludeIdentifierSuffix = "\"";
  std::ifstream file(file_path);
  CGCHECK(file.is_open()) << " Cannot open file (Reason1 FilePath. Reason2 Recursive include) : include_path~"
                          << file_path << " compiling_name~" << name_;
  std::string line;
  int line_num = 0;
  std::string file_content;
  std::vector<std::string> dependances;
  while (std::getline(file, line)) {
    bool is_comment = util::StartsWith(util::TrimLeft(line), kComment);
    int prefix_start = line.find(kIncludeIdentifierPrefix);
    bool is_include = prefix_start != std::string::npos;
    if (is_include && !is_comment) {
      int dependance_start = prefix_start + kIncludeIdentifierPrefix.size();
      int dependance_end = line.find(kIncludeIdentifierSuffix, dependance_start);
      CGCHECK(dependance_end != std::string::npos) << " No Suffix to finish Prefix : file_path~" << file_path << " name:" << name_;
      int dependance_length = dependance_end - dependance_start;
      std::string dependance = line.substr(dependance_start, dependance_length);

      CGCHECK(dependance != file_path) << " Must not include self : file_path~" << file_path;
      ParseAFile(dependance);
      dependances.push_back(dependance);

      for (int i = 0; i <= dependance_end; ++i) {
        line[i] = ' ';
      }
    }
    file_content += line + '\n';
    line_num++;
  }
  CGCHECK(file_content.size() > 0) << "Empty file : file_path~" << file_path << " name~" << name_;
  CGCHECK(file_content != "") << "Content should not empty : file_path~" << file_path << " name~" << name_;

  file_meta_map_[file_path].line_num = line_num;
  file_meta_map_[file_path].content = file_content;
  file_meta_map_[file_path].dependances = dependances;

  file.close();
}

std::vector<std::string> ShaderParser::TopologicalSort() {
  std::vector<std::string> res;
  std::set<std::string> sorted;
  while (res.size() < file_meta_map_.size()) {
    bool found = false;
    std::string no_dependance;
    for (auto& p : file_meta_map_) {
      const FileMeta& meta = p.second;
      if (sorted.find(p.first) == sorted.end() && meta.dependances.size() == 0) {
        no_dependance = p.first;
        res.push_back(no_dependance);
        found = true;
        break;
      }
    }
    for (auto iter = file_meta_map_.begin(); iter != file_meta_map_.end(); ++iter) {
      util::Remove(no_dependance, &iter.value().dependances);
    }
    CGCHECK(found) << " Circular dependance found !";
    sorted.insert(no_dependance);
  }
  return res;
}

ShaderProgram ShaderLoader::Load(const std::string& name, const std::unordered_map<FileType, std::string>& file_paths) {
  bool is_render_shader = (file_paths.at(kVS) != "" && file_paths.at(kFS) != "");
  bool is_compute_shader = (file_paths.at(kCS) != "");
  CGCHECK(is_render_shader || is_compute_shader) << " Must render shader or compute shader : " << name;
  CGCHECK(!(is_render_shader && is_compute_shader)) << " Must not render shader && compute shader : " << name;

  std::vector<ShaderProgram::CodePart> vs, fs, gs, ts, cs;
  std::unordered_map<FileType, std::vector<ShaderProgram::CodePart>*> type_storage_map =
      {{kVS, &vs}, {kFS, &fs}, {kGS, &gs}, {kTS, &ts}, {kCS, &cs}};
  for (auto& p : type_storage_map) {
    if (file_paths.at(p.first) != "") {
      ShaderParser parser(name);
      std::vector<ShaderProgram::CodePart> code_parts = parser.Parse(file_paths.at(p.first));
      *p.second = code_parts;
    }
  }
  if (is_render_shader) {
    return ShaderProgram(name, vs, fs, gs, ts);
  } else if (is_compute_shader) {
    return ShaderProgram(name, cs);
  } else {
    CGCHECK(false) << "What shader ?";
    return ShaderProgram();
  }
}

} // namespace renderer