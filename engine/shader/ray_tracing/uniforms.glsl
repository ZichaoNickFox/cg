#include "engine/shader/bvh.glsl"
#include "engine/shader/camera.glsl"
#include "engine/shader/geometry.glsl"

uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;

// Texture binding
#define TEXTURE_BINDING_CANVAS 0
layout (rgba32f, binding = TEXTURE_BINDING_CANVAS) uniform image2D canvas;

#define SSBO_BINDING_LIGHT_PATH 0
#define SSBO_BINDING_BVN_NODES 1
#define SSBO_BINDING_TRIANGLES 2
layout (std430, binding = SSBO_BINDING_LIGHT_PATH) buffer LightPath { vec4 light_path[]; };
layout (std430, binding = SSBO_BINDING_BVN_NODES) buffer BVHNodes { BVHNode bvh_nodes[]; };
layout (std430, binding = SSBO_BINDING_TRIANGLES) buffer Triangles { Triangle triangles[]; };

