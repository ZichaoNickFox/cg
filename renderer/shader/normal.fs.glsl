#include "renderer/shader/version.glsl"

out vec4 FragColor;

in vec4 gs_output_color_;

void main()
{
  FragColor = gs_output_color_;
}