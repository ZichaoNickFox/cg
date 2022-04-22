uniform int light_count;
uniform Light lights[200];

struct PhongModelInput {
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
  vec3 result = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < light_count; ++i) {
    Light light = lights[i];

    // ambient
    vec3 ambient_component = light.color * param.ambient;

    // diffuse
    vec3 L = normalize(light.pos - param.frag_pos_ws);
    vec3 N = param.normal;
    float diff_factor = max(dot(N, L), 0.0);
    vec3 diffuse_component = light.color * diff_factor * param.diffuse;

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
    vec3 specular_component = light.color * spec_factor * param.specular;

    float dist = length(light.pos - param.frag_pos_ws);
    float attenuation = 1.0 / (dist * dist * light.quadratic + dist * light.linear + light.constant);

    result += (ambient_component + diffuse_component + specular_component) * attenuation;
  }
  return result;
}