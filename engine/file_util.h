#pragma once

#include <string>

namespace engine {
namespace file_util {

void ReadFileToString(const std::string& path, std::string* content);
std::string GetFileExt(const std::string& file_name);

}
}