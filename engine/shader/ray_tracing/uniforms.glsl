#include "engine/shader/bvh.glsl"
#include "engine/shader/camera.glsl"
#include "engine/shader/geometry.glsl"

uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;

layout (rgba32f, binding = 0) uniform image2D canvas;

layout (std430, binding = 0) buffer LightPath { vec4 light_path[]; };
layout (std430, binding = 1) buffer BVHNodes { BVHNode bvh_nodes[]; };
layout (std430, binding = 2) buffer Triangles { Triangle triangles[]; };

