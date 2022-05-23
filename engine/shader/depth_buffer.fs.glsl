out vec4 FragColor;

void main() { 
  float depth = GetLinearDepth(u_near, u_far);
  FragColor = vec4(vec3(depth), 1.0);
}