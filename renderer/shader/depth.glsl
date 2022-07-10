float NormalizedDepthVS(float depth_vs, float near, float far) {
  float normalized_linear_depth_vs = (-depth_vs - near) / (far - near);
  return normalized_linear_depth_vs;
}

float DepthSS2VS(float z_ss, float near, float far) {
  float z = z_ss * 2.0 - 1.0;
  float neg_depth_vs = (2.0 * near * far) / (far + near - z * (far - near));
  return -neg_depth_vs;
}