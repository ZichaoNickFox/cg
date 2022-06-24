#version 430 core

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D texture_ambient;
uniform sampler2D texture_height;
uniform sampler2D texture_albedo;
uniform sampler2D texture_metallic;
uniform sampler2D texture_roughtness;
uniform bool use_texture_diffuse = false;
uniform bool use_texture_specular = false;
uniform bool use_texture_normal = false;
uniform bool use_texture_ambient = false;
uniform bool use_texture_height = false;
uniform bool use_texture_albedo = false;
uniform bool use_texture_metallic = false;
uniform bool use_exture_roughtness = false;

in vec2 texcoord_;
out vec4 color;

void main() {
  if (use_texture_diffuse) {
    color = texture(texture_diffuse, texcoord_);
  } else if (use_texture_albedo) {
    color = texture(texture_albedo, texcoord_);
  }
}
