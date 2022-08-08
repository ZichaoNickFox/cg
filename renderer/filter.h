#pragma once

#include <string>
#include <vector>

namespace cg {

struct Filter {
 public:
  enum Type {
    kNone,
    kIncludes,
    kExcludes
  };
  Filter(Type type = kNone, const std::vector<std::string>& filters = {}) : type_(type), filters_(filters) {}
  bool Pass(const std::string& judgement) const;

 private:
  std::vector<std::string> filters_;
  Type type_;
};

} // namespace cg