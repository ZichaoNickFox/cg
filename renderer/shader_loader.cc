#include "renderer/shader_loader.h"

#include <filesystem>
#include <utility>

#include "base/util.h"

namespace cg {
namespace {

std::string NormalizePath(const std::filesystem::path& path) {
  return util::ReplaceBackslash(std::filesystem::weakly_canonical(path).string());
}

}  // namespace

ShaderParser::ShaderParser(const std::string& name) {
  name_ = name;
}

std::vector<ShaderCodePart> ShaderParser::Parse(const std::string& file_path) {
  file_meta_map_.clear();
  active_parse_stack_.clear();
  ParseAFile(ResolvePath(file_path, ""));

  std::vector<std::string> sorted = TopologicalSort();
  std::vector<ShaderCodePart> res;
  for (const std::string& path : sorted) {
    res.push_back({path, file_meta_map_[path].content});
  }
  return res;
}

void ShaderParser::ParseAFile(const std::string& file_path) {
  if (file_meta_map_.find(file_path) != file_meta_map_.end()) {
    return;
  }
  CGCHECK(active_parse_stack_.count(file_path) == 0) << " Circular include detected : file_path~"
                                                     << file_path << " compiling_name~" << name_;

  const std::string kComment = "//";
  const std::string kIncludeIdentifierPrefix = "#include \"";
  const std::string kIncludeIdentifierSuffix = "\"";
  std::ifstream file(file_path);
  CGCHECK(file.is_open()) << " Cannot open shader file : file_path~" << file_path << " compiling_name~" << name_;

  active_parse_stack_.insert(file_path);
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
      std::string dependance = ResolvePath(line.substr(dependance_start, dependance_length), file_path);

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
  active_parse_stack_.erase(file_path);
}

std::string ShaderParser::ResolvePath(const std::string& requested_path, const std::string& including_file_path) const {
  const std::filesystem::path raw_path(requested_path);
  std::vector<std::filesystem::path> candidates;
  if (raw_path.is_absolute()) {
    candidates.push_back(raw_path);
  } else {
    if (!including_file_path.empty()) {
      candidates.push_back(std::filesystem::path(including_file_path).parent_path() / raw_path);
    }
    candidates.push_back(raw_path);
    candidates.push_back(std::filesystem::path(CG_PROJECT_SOURCE_DIR) / raw_path);
  }

  for (const std::filesystem::path& candidate : candidates) {
    std::error_code error;
    if (std::filesystem::exists(candidate, error) && !error) {
      return NormalizePath(candidate);
    }
  }

  std::string attempted_paths;
  for (size_t i = 0; i < candidates.size(); ++i) {
    if (i > 0) {
      attempted_paths += ", ";
    }
    attempted_paths += util::ReplaceBackslash(candidates[i].string());
  }
  CGCHECK(false) << " Cannot resolve shader include path : requested_path~" << requested_path
                 << " including_file~" << including_file_path
                 << " attempted_paths~[" << attempted_paths << "]"
                 << " compiling_name~" << name_;
  return requested_path;
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

ShaderProgramDesc ShaderLoader::LoadProgramDesc(const std::string& name,
                                                const std::unordered_map<FileType, std::string>& file_paths) {
  bool is_render_shader = (file_paths.at(kVS) != "" && file_paths.at(kFS) != "");
  bool is_compute_shader = (file_paths.at(kCS) != "");
  CGCHECK(is_render_shader || is_compute_shader) << " Must render shader or compute shader : " << name;
  CGCHECK(!(is_render_shader && is_compute_shader)) << " Must not render shader && compute shader : " << name;

  std::vector<ShaderCodePart> vs, fs, gs, ts, cs;
  std::unordered_map<FileType, std::vector<ShaderCodePart>*> type_storage_map =
      {{kVS, &vs}, {kFS, &fs}, {kGS, &gs}, {kTS, &ts}, {kCS, &cs}};
  for (auto& p : type_storage_map) {
    if (file_paths.at(p.first) != "") {
      ShaderParser parser(name);
      std::vector<ShaderCodePart> code_parts = parser.Parse(file_paths.at(p.first));
      *p.second = code_parts;
    }
  }
  if (is_render_shader) {
    return {
        .name = name,
        .kind = rhi::ProgramKind::kRender,
        .vs = std::move(vs),
        .fs = std::move(fs),
        .gs = std::move(gs),
        .ts = std::move(ts),
    };
  }
  if (is_compute_shader) {
    return {
        .name = name,
        .kind = rhi::ProgramKind::kCompute,
        .cs = std::move(cs),
    };
  }
  CGCHECK(false) << "What shader ?";
  return {};
}

ShaderProgram ShaderLoader::Load(const std::string& name, const std::unordered_map<FileType, std::string>& file_paths) {
  return ShaderProgram(LoadProgramDesc(name, file_paths));
}

} // namespace cg
