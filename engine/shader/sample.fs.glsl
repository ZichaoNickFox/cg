out vec4 FragColor;

in vec3 local_pos_;

void main() {
  const float pi = 3.1415926;
  const uint N = 100;
  FragColor = vec4(0, 0, 1, 1);
  for (int i = 0; i < N; ++i) {
    vec2 s = Hammersley(i, N);
    vec2 uv = SpherePos2UV(local_pos_);
    if (distance(s, uv) < 0.005) {
      FragColor  = vec4(1, 0, 0, 1);
    }
  }
}