#version 330 core

out vec4 FragColor;
in vec2 texcoord_;

uniform sampler2D scene;
uniform sampler2D bright;

vec3 blur() {
  vec3 res = vec3(0, 0, 0);

  const int n_directions = 10;
  
  const int n_points = 10;
  float weights[n_points];
  for (int i = 0; i < n_points; ++i) {
    weights[i] = 1.0 / n_points;
  }

  float point_interval = 1;
  vec2 uv_size = 1.0 / textureSize(bright, 0);
  float pi2 = 3.1415926 * 2;
  for (int direction = 0; direction < n_directions; direction += 1) {
    float angle = pi2 / n_directions * direction;
    vec3 direction_color = vec3(0, 0, 0);
    for (int point = 0; point < n_points; ++point) {
      vec2 sample_uv = texcoord_ + vec2(uv_size.x * point_interval * point * cos(angle),
                                        uv_size.y * point_interval * point * sin(angle));
      sample_uv = clamp(sample_uv, 0, 1);
      direction_color += (texture(bright, sample_uv) * weights[point]).xyz;
    }
    res += direction_color / n_directions;
  }
  return res;
}

void main() {
  vec4 scene_color = texture(scene, texcoord_);
  vec4 bright_color = vec4(blur(), 0);
  FragColor = scene_color + bright_color;
}