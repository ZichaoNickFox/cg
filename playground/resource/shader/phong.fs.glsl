#version 330 core

out vec4 FragColor;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};
uniform Material material;

struct Light {
  vec3 color;
  vec3 pos;

  // attenuation
  float constant;
  float linear;
  float quadratic;
};
uniform int light_count;
uniform Light lights[200];
uniform vec3 view_pos;
uniform bool blinn_phong;

in vec3 normal_;
in vec3 frag_world_pos_;

vec3 CalcLight(Light light) {
  // ambient
  vec3 ambient = light.color * material.ambient;

  // diffuse
  vec3 norm = normalize(normal_);
  vec3 frag_2_light = normalize(light.pos - frag_world_pos_);
  float diff_factor = max(dot(norm, frag_2_light), 0.0);
  vec3 diffuse = light.color * diff_factor * material.diffuse;

  // specular
  vec3 frag_2_view = normalize(view_pos - frag_world_pos_);
  vec3 reflect_dir = reflect(-frag_2_light, norm);
  vec3 half_dir = normalize(frag_2_light + frag_2_view);
  float spec_factor = 0.0;
  if (blinn_phong) {
    spec_factor = pow(max(dot(norm, half_dir), 0.0), material.shininess * 2.0);
  } else {
    spec_factor = pow(max(dot(frag_2_view, reflect_dir), 0.0), material.shininess);
  }
  vec3 specular = light.color * spec_factor * material.specular;

  float dist = length(light.pos - frag_world_pos_);
  float attenuation = 1.0 / (dist * dist * light.quadratic + dist * light.linear + light.constant);

  vec3 result = (ambient + diffuse + specular) * attenuation;
  return result;
}

void main()
{
  // lighting
  vec3 light_result = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < light_count; ++i) {
    light_result += CalcLight(lights[i]);
  }
  FragColor = vec4(light_result, 1.0);
}