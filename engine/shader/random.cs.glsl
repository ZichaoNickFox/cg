layout (local_size_x = 32, local_size_y = 32) in;
// https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
// Only suport Not support rgb32f, 
layout (rgba32f, binding = 0) uniform image2D input_texture;
layout (rgba32f, binding = 1) uniform image2D output_texture;

void main() {
  ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
  float r = random_cs(0);
  vec4 color = vec4(r, r, r, 1);
  imageStore(output_texture, pos, color);
}