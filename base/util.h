#pragma once

#include <chrono>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/debug.h"

namespace util {
// file
void ReadFileToString(const std::string& path, std::string* content);
std::string FileExt(const std::string& file_name);
std::string FileName(const std::string& path);
std::string FileDir(const std::string& path);
std::string ReplaceBackslash(const std::string& path);
void MakeDir(const std::string& dir);
std::string FileJoin(const std::string& dir, const std::string& file);

// string
bool StartsWith(const std::string& str, const std::string& start_with);
bool EndsWith(const std::string& str, const std::string& end_with);
template<typename PtrType>
std::string AsString(PtrType* ptr) {
  return Format("{}", uint64_t(ptr));
}
std::string TrimLeft(std::string str);
std::string TrimRight(std::string str);
std::string Trim(std::string str);

// stl
template<typename ElemType>
uint64_t VectorSizeInByte(const std::vector<ElemType>& v) {
  return sizeof(ElemType) * v.size();
}
template<int N, typename ElemType>
std::array<ElemType, N> AsArray(const std::vector<ElemType>& in) {
  std::array<ElemType, N> res;
  std::copy_n(in.begin(), N, res.begin());
  return res;
}
template<typename Type>
const void* AsVoidPtr(const Type& var) {
  return reinterpret_cast<const void*>(var);
}
template<typename KeyType, typename ValueType>
std::vector<ValueType> AsValueVector(const std::unordered_map<KeyType, ValueType>& in) {
  std::vector<ValueType> res;
  for (auto& pair : in) {
    res.push_back(pair.second);
  }
  return res;
}
template<typename KeyType, typename ValueType>
std::vector<KeyType> AsKeyVector(const std::unordered_map<KeyType, ValueType>& in) {
  std::vector<KeyType> res;
  for (auto& pair : in) {
    res.push_back(pair.first);
  }
  return res;
}
template <typename ArrayElemType, int N>
std::vector<ArrayElemType> AsVector(const ArrayElemType (&array)[N]) {   
 int num = (sizeof(array) / sizeof(array[0])); 
 return {array, array + num};
};
template<typename VectorElemType>
std::vector<VectorElemType> SubVector(const std::vector<VectorElemType>& source, int begin, int end) {
  return {source.begin() + begin, source.begin() + end};
}
template<typename VectorElemType>
void Remove(const VectorElemType& elem, std::vector<VectorElemType>* elems) {
  for (auto iter = elems->begin(); iter != elems->end();) {
    if (elem == *iter) {
      iter = elems->erase(iter);
    } else {
      iter++;
    }
  }
}
template<typename ElemType>
void VectorOverride(std::vector<ElemType>* destination, int destination_begin,
                    const std::vector<ElemType>& source, int source_begin, int source_end) {
  int len = source_end - source_begin; 
  if (destination->size() - destination_begin < len) {
    destination->resize(destination_begin + len);
  }
  memcpy(destination->data() + destination_begin, source.data() + source_begin, len * sizeof(ElemType));
}

// time
using Time = std::chrono::high_resolution_clock::time_point;
Time Now();
int64_t DurationMillisecond(const Time& from, const Time& to);
float AsFloat(const Time& time);
int AsInt(const Time& time);

// float
float FloatEq(float value, float target);

// glm
glm::ivec2 operator*(const glm::ivec2& left, double value);

}