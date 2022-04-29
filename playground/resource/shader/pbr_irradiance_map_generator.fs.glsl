#version 410 core

uniform samplerCube cubemap;

out vec4 FragColor;

in vec3 normal_;

void main() {
  vec3 irradiance = vec3(0.0);
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = cross(up, normal_);
  up = cross(normal_, right);
  const float pi = 3.1415926;

  float sample_delta = 0.025;
  float nr_samples = 0.0;
  for (float phi = 0.0; phi < 2.0 * pi; phi += sample_delta) {
    for (float theta = 0.0; theta < pi / 2.0; theta += sample_delta) {
      vec3 sample_tangent_space = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      vec3 sample_local_space = sample_tangent_space.x * right + sample_tangent_space.y * up +
                                sample_tangent_space.z * normal_;
      irradiance += texture(cubemap, sample_local_space).xyz * cos(theta) * sin(theta);
      nr_samples++;
    }
  }
  irradiance = pi * irradiance / nr_samples;
  FragColor = vec4(irradiance, 1.0);
}