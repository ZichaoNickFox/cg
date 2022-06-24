#include "renderer/shader/definition.glsl"

struct Light {
  vec4 type_pos; // dirctional_light = 0, point_light = 1, spot_light = 2
  vec4 color;
  vec4 attenuation_2_1_0; // free w
};

uniform int light_repo_length;
layout (std430, binding = SSBO_LIGHT_REPO) buffer LightRepo {
  Light light_repo[];
};

int LightType(Light light) {
  return int(light.type_pos.x);
}

vec3 LightPos(Light light) {
  return light.type_pos.yzw;
}

vec4 LightColor(Light light) {
  return light.color;
}

float LightQuadratic(Light light) {
  return light.attenuation_2_1_0.x;
}

float LightLinear(Light light) {
  return light.attenuation_2_1_0.y;
}

float LightConstant(Light light) {
  return light.attenuation_2_1_0.z;
}