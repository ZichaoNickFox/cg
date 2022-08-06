#include <eigen/core>
#include <eigen/Dense>
#include <vector>

namespace eigen = Eigen;

inline eigen::VectorXd Vector2Eigen(std::vector<double> v) {
  return eigen::Map<eigen::VectorXd>(v.data(), v.size());
}

inline eigen::VectorXf Vector2Eigen(std::vector<float> v) {
  return eigen::Map<eigen::VectorXf>(v.data(), v.size());
}

// template <typename ArrayElemType, int N>
// eigen::VectorXd Array2Eigen(const ArrayElemType (&array)[N]) {   
//   int num = sizeof(array) / sizeof(array[0]); 
//   return eigen::Map<eigen::VectorXd>(&array, num);
// }