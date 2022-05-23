uniform float u_far;
uniform float u_near;

float LinearDepth(float depth_ss, float near, float far) {
  float z = depth_ss * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - z * (far - near));
}

float GetLinearDepth(float near, float far) {
  return LinearDepth(gl_FragCoord.z, near, far) / (far - near);
}
