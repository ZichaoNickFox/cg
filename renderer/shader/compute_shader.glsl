#include "renderer/shader/definition.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = IMAGE_CANVAS) uniform image2D canvas;