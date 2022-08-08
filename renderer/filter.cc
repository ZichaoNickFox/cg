#include "renderer/filter.h"

namespace cg {

bool Filter::Pass(const std::string& judgement) const {
  if (type_ == kNone) {
    return true;
  }
  bool found = (std::find(filters_.begin(), filters_.end(), judgement) != filters_.end());
  if (type_ == kIncludes) {
    return found;
  } else {
    return !found;
  }
}

} // namespace cg