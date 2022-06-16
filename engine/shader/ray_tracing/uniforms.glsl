#include "engine/shader/geometry.glsl"

uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;

// Texture binding
const int texture_binding_canvas = 0;
layout (rgba32f, binding = texture_binding_canvas) uniform image2D canvas;

// SSBO binding
const int ssbo_binding_light_path = 0;
const int ssbo_binding_bvn_nodes = 1;
const int ssbo_binding_triangles = 2;
layout (std430, binding = ssbo_binding_light_path) buffer LightPath { vec4 light_path[]; };
