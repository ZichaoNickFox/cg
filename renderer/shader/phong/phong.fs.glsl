#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/material.glsl"
#include "renderer/shader/light.glsl"
#include "renderer/shader/phong/phong_model.glsl"
#include "renderer/shader/texture.glsl"

out vec4 FragColor;

uniform int material_index;
uniform bool use_blinn_phong;
uniform Camera camera;

in mat3 TBN_ws_;
in vec2 texcoord_;
in vec3 position_ws_;
in vec3 normal_ws_;

void main()
{
  Material material = material_repo[material_index];

  PhongModelInput phong_model_input;
  phong_model_input.ambient = MaterialAmbient(material, texcoord_);
  phong_model_input.diffuse = MaterialDiffuse(material, texcoord_);
  phong_model_input.normal = MaterialNormal(material, TBN_ws_, normal_ws_, texcoord_);
  phong_model_input.specular = MaterialSpecular(material, texcoord_);
  phong_model_input.shininess = MaterialShininess(material, texcoord_);
  phong_model_input.frag_pos_ws = position_ws_;
  phong_model_input.view_pos_ws = camera.pos_ws;
  phong_model_input.use_blinn_phong = use_blinn_phong;

  FragColor = PhongModel(phong_model_input);
}