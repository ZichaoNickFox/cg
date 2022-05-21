#pragma once

#include <memory>
#include <string>

#include "engine/debug.h"

namespace engine {
class Any {
 public:
  template<typename DataType>
  explicit Any(const DataType& data) {
    CHECK(!data_) << "Don't assign value twice";
    data_ = new Data<DataType>(data);
  }
  template<typename DataType>
  explicit Any(DataType&& data) {
    CHECK(!data_) << "Don't assign value twice";
    data_ = new Data<DataType>(data);
  }
  explicit Any(const Any& other) {
    CHECK(!data_) << "Don't assign value twice";
    data_ = other.data_->Clone();
  }

  template<typename DataType>
  bool TypeIs() const { return typeid(DataType) == data_->Type(); }

  template<typename DataType>
  const DataType& Value() const {
    CGCHECK(TypeIs<DataType>());
    return static_cast<Data<DataType>*>(data_)->data;
  }

  template<typename DataType>
  DataType& MutableValue() {
    CGCHECK(TypeIs<DataType>());
    return static_cast<Data<DataType>*>(data_)->data;
  }

  std::string TypeName() {
    return data_->Type().name();
  }

 protected:
  struct DataBase {
    virtual const std::type_info& Type() const = 0;
    virtual DataBase* Clone() const = 0;
  };
  template<typename DataType>
  struct Data : DataBase {
    explicit Data(const DataType& in) : data(in) {}
    virtual const std::type_info& Type() const {
      return typeid(data);
    }
    virtual DataBase* Clone() const { return new Data<DataType>(data); }
    DataType data;
  };
  DataBase* data_ = nullptr;
};
} // namespace engine
