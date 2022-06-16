#version 430 core

uniform sampler2D texture0;
uniform int sample_frame_num;

in vec2 texcoord_;
out vec4 FragColor;

void main()
{
  FragColor = texture(texture0, texcoord_);
  if (sample_frame_num > 0) {
    FragColor = FragColor /  sample_frame_num;
  }
}