#include "engine/file_util.h"

#include <fstream>
#include <glog/logging.h>
#include <string>

namespace engine {
namespace file_util {

void ReadFileToString(const std::string& path, std::string* content) {
  CHECK(content);
  std::ifstream input_file(path);
  *content = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
  input_file.close();
}

std::string GetFileExt(const std::string& file_name) {
  int pos = file_name.find_last_of('.');
  if (pos == std::string::npos) {
    return "";
  }
  return file_name.substr(pos + 1);
}

}
}