struct PhongModelInput {
  vec3 light_color;
  vec3 light_pos_ws;
  float light_quadratic;
  float light_linear;
  float light_constant;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 normal;
  float shininess;

  vec3 frag_pos_ws;
  vec3 view_pos_ws;

  bool blinn_phong;
};

vec3 PhongModel(PhongModelInput param) {
  // ambient
  vec3 ambient_component = param.light_color * param.ambient;

  // diffuse
  vec3 L = normalize(param.light_pos_ws - param.frag_pos_ws);
  vec3 N = param.normal;
  float diff_factor = max(dot(N, L), 0.0);
  vec3 diffuse_component = param.light_color * diff_factor * param.diffuse;

  // specular
  vec3 V = normalize(param.view_pos_ws - param.frag_pos_ws);
  vec3 R = reflect(-L, N);
  float spec_factor = 0.0;
  if (param.blinn_phong) {
    vec3 H = normalize(L + V);
    spec_factor = pow(max(dot(N, H), 0.0), param.shininess * 2.0);
  } else {
    spec_factor = pow(max(dot(V, R), 0.0), param.shininess);
  }
  vec3 specular_component = param.light_color * spec_factor * param.specular;

  float dist = length(param.light_pos_ws - param.frag_pos_ws);
  float attenuation = 1.0 / (dist * dist * param.light_quadratic + dist * param.light_linear + param.light_constant);

  vec3 result = (ambient_component + diffuse_component + specular_component) * attenuation;
  return result;
}