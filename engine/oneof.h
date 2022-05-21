#pragma once

#include <type_traits>

#include "engine/any.h"

namespace engine {
template<typename ...>
struct IsInTypes {
  static constexpr bool value = false;
};

template<typename ValueType, typename HeadType, typename ... OtherOptionalTypes>
struct IsInTypes<ValueType, HeadType, OtherOptionalTypes ...> {
  static constexpr bool value = std::is_same<ValueType, HeadType>::value
      || IsInTypes<ValueType, OtherOptionalTypes ...>::value;
};

template<typename ... OptionalTypes>
class Oneof : protected Any {
 private:
  typedef Any Super;

 public:
  template<typename DataType>
  explicit Oneof(DataType&& data) : Super(data) {
    static_assert(IsInTypes<DataType, OptionalTypes ...>::value, "Value's type must be one of optional types");
  }
  template<typename DataType>
  explicit Oneof(const DataType& data) : Super(data) {
    static_assert(IsInTypes<DataType, OptionalTypes ...>::value, "Value's type must be one of optional types");
  }

  using Super::TypeIs;
  using Super::Value;
  using Super::MutableValue;
  using Super::TypeName;
};
} // namespace engine
