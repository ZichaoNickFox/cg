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

vec3 PositionLS2WS(mat4 model, vec3 position_ls) {
  return vec3(model * vec4(position_ls, 1.0));
}

vec3 PositionWS2VS(mat4 view, vec3 position_ws) {
  return vec3(view * vec4(position_ws, 1.0));
}

// pos_ss must in [0, 1]
vec3 PositionSS2WS(mat4 view, mat4 project, vec3 pos_ss) {
  vec3 pos_ns = pos_ss * vec3(2.0) - vec3(1.0);
  mat4 inverse_vp = inverse(project * view);
  vec4 pos_ws = inverse_vp * vec4(pos_ns, 1.0);
  pos_ws = pos_ws / pos_ws.w;
  return pos_ws.xyz;
}

vec3 PositionWS2SS(mat4 view, mat4 project, vec3 position_ws) {
  vec4 pos_cs = project * view * vec4(position_ws, 1.0);
  return (pos_cs / pos_cs.w * 0.5 + 0.5).xyz;
}

vec4 PositionLS2CS(mat4 model, mat4 view, mat4 project, vec3 position_ls) {
  return project * view * model * vec4(position_ls, 1.0);
}

vec4 PositionWS2CS(mat4 view, mat4 project, vec3 position_ws) {
  return project * view * vec4(position_ws, 1.0);
}

vec4 PositionWS2CS(mat4 view_project, vec3 position_ws) {
  return view_project * vec4(position_ws, 1.0);
}

vec3 SurfaceNormal(vec3 position1_ws, vec3 position2_ws, vec3 position3_ws) {
  return normalize(cross(position2_ws - position1_ws, position3_ws - position1_ws));
}