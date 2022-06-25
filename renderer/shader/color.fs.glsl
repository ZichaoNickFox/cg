#include "renderer/shader/version.glsl"

uniform vec4 color;
out vec4 FragColor;

void main()
{
  FragColor = color;
}