#version 410 core

uniform sampler2D u_texture_input;
uniform vec2 u_viewport_size;

in vec2 texcoord_;
out vec4 out_color;

void main() {
  int dir_num = 6;
  int sample_per_dir = 2;
  int unit_interval = 2;
  float unit_angle = 2 * 3.1415926 / dir_num;
  vec2 per_pixel = vec2(1.0) / u_viewport_size;
  out_color = texture(u_texture_input, texcoord_);
  for (int iter_dir = 0; iter_dir < dir_num; ++iter_dir) {
    for (int iter_sample = 1; iter_sample <= sample_per_dir; ++iter_sample) {
      float angle = iter_dir * unit_angle;
      vec2 interval = vec2(cos(angle), sin(angle));
      vec2 sample_texcoord = texcoord_ + interval * iter_sample * unit_interval / u_viewport_size;
      out_color += texture(u_texture_input, sample_texcoord);
    }
  }
  out_color /= (1 + dir_num * sample_per_dir);
}