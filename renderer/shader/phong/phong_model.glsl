#include "renderer/shader/light.glsl"

struct PhongModelInput {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec3 normal;
  float shininess;

  vec3 frag_pos_ws;
  vec3 view_pos_ws;

  bool use_blinn_phong;
};

vec4 PhongModel(PhongModelInput param) {
  vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
  for (int i = 0; i < light_repo_length; ++i) {
    Light light = light_repo[i];

    // ambient
    vec4 ambient_component = LightColor(light) * param.ambient;

    // diffuse
    vec3 L = normalize(LightPos(light) - param.frag_pos_ws);
    vec3 N = param.normal;
    float diffuse_factor = max(dot(N, L), 0.0);
    vec4 diffuse_component = LightColor(light) * diffuse_factor * param.diffuse;

    // specular
    vec3 V = normalize(param.view_pos_ws - param.frag_pos_ws);
    vec3 R = reflect(-L, N);
    float spec_factor = 0.0;
    if (param.use_blinn_phong) {
      vec3 H = normalize(L + V);
      spec_factor = pow(max(dot(N, H), 0.0), param.shininess * 2.0);
    } else {
      spec_factor = pow(max(dot(V, R), 0.0), param.shininess);
    }
    vec4 specular_component = LightColor(light) * spec_factor * param.specular;

    float dist = length(LightPos(light) - param.frag_pos_ws);
    float attenuation = 1.0 / (dist * dist * LightQuadratic(light) + dist * LightLinear(light) + LightConstant(light));
    result += (ambient_component + diffuse_component + specular_component) * attenuation;
  }
  return result;
}