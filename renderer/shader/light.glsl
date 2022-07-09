#include "renderer/shader/definition.glsl"

#include "renderer/shader/color.glsl"
#include "renderer/shader/material.glsl"
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
uniform float primitive_light_area;
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
  Primitive primitive = primitive_repo[primitive_index];
  Material material = material_repo[PrimitiveMaterialIndex(primitive)];
  return MaterialEmission(material) != vec4(0, 0, 0, 1);
}

float LightAttenuation(Light light, float distance) {
  return 1.0 / (distance * distance * LightQuadratic(light) + distance * LightLinear(light)
      + LightConstant(light));
}

Light GetPrimitiveLight(int primitive_light_index) {
  for (int i = 0, j = 0; i < light_repo_num; ++i) {
    Light light = light_repo[i];
    if (LightType(light) == 0) {
      if (j == primitive_light_index) {
        return light;
      }
      j++;
    }
  }
  return Light(kColorError, kColorError, kColorError, kColorError);
}