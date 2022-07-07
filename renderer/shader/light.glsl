#include "renderer/shader/definition.glsl"

#include "renderer/shader/primitive.glsl"

// primitive_light = 0
// dirctional_light = 1
// point_light = 2
// spot_light = 3
struct Light {
  vec4 type_primitiveindex;
  vec4 position;
  vec4 color;
  vec4 attenuation_2_1_0; // free w
};

uniform int light_repo_num;
uniform int primitive_light_num;
uniform int primitive_light_area;
layout (std430, binding = SSBO_LIGHT_REPO) buffer LightRepo {
  Light light_repo[];
};

int LightType(Light light) {
  return int(light.type_primitiveindex.x);
}

int LightPrimitiveIndex(Light light) {
  return int(light.type_primitiveindex.y);
}

vec3 LightPosition(Light light) {
  return light.position.xyz;
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

bool IsLight(int primitive_index) {
  for (int i = 0; i < light_repo_num; ++i) {
    Light light = light_repo[i];
    if (LightPrimitiveIndex(light) == primitive_index) {
      return true;
    }
  }
  return false;
}