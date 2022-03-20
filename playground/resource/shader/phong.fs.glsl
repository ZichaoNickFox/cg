#version 330 core

out vec4 FragColor;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};
uniform Material material;

uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

in vec3 normal_;
in vec3 frag_pos_;

// http://www.barradeau.com/nicoptere/dump/materials.html
void main()
{
  // ambient
  vec3 ambient = light_color * material.ambient;

  // diffuse
  vec3 norm = normalize(normal_);
  vec3 light_dir = normalize(light_pos - frag_pos_);
  float diff_factor = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light_color * diff_factor * material.diffuse;

  // specular
  vec3 view_dir = normalize(view_pos - frag_pos_);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec_factor = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light_color * spec_factor * material.specular;

  vec3 result = ambient + diffuse + specular;
  FragColor = vec4(result, 1.0);
}