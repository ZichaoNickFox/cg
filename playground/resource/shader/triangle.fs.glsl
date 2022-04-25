#version 410 core

uniform sampler2D texture0;
uniform sampler2D texture1;

in vec2 texcoord_;
in vec3 color_;
out vec4 FragColor;

void main()
{
  vec4 color0 = texture(texture0, texcoord_);
  vec4 color1 = texture(texture1, texcoord_);
  float x = 0.3 * (color_.x + color0.x + color1.x);
  float y = 0.3 * (color_.y + color0.y + color1.y);
  float z = 0.3 * (color_.z + color0.z + color1.z);
  FragColor = vec4(x, y, z, 1.0f);
}