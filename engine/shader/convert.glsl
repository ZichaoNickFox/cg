// http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
vec3 NormalLS2WS(mat4 model, vec3 normal_ls) {
  mat3 normal_matrix = transpose(inverse(mat3(model)));
  return normalize(normal_matrix * normal_ls);
}

vec3 NormalLS2VS(mat4 model, mat4 view, vec3 normal_ls) {
  mat3 normal_matrix = transpose(inverse(mat3(view * model)));
  return normalize(normal_matrix * normal_ls);
}

vec3 NormalWS2VS(mat4 view, vec3 normal_ws) {
  mat3 normal_matrix = transpose(inverse(mat3(view)));
  return normalize(normal_matrix * normal_ws);
// The follow one seems OK too.
//  return vec3(view * vec4(normal_ws, 1.0));
}

vec3 PositionWS2VS(mat4 view, vec3 position_ws) {
  return vec3(view * vec4(position_ws, 1.0));
}