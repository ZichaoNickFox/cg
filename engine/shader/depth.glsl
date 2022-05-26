// Linear Depth is negative view space z. Range [near, far]
float GetLinearDepthFromSS(float depth_ss, float near, float far) {
  float z = depth_ss * 2.0 - 1.0;
  float depth_vs = (2.0 * near * far) / (far + near - z * (far - near));
  return depth_vs;
}

// Linear Depth is negative view space z. Range [near, far]
float GetLinearDepthFromVS(float z_vs) {
  return -z_vs;
}

// Linear Depth is negative view space z. After normalize, Range [0, 1]
float NormalizeLinearDepth(float depth_vs, float near, float far) {
  float normalized_linear_depth_vs = (depth_vs - near) / (far - near);
  return normalized_linear_depth_vs;
}
