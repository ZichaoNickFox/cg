out vec4 FragColor;
in vec3 local_pos_;

uniform sampler2D texture0;

void main() {
  // After normalize, local_pos coverted to inscribed sphere
  vec2 uv = GetEquirectangularTexcoord(normalize(local_pos_));
  vec3 color = texture(texture0, uv).rgb;
  FragColor = vec4(color, 1.0);
}