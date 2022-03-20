#version 330 core

uniform vec3 light_color;
out vec4 FragColor;

// http://www.barradeau.com/nicoptere/dump/materials.html
void main()
{
  FragColor = vec4(light_color, 1.0f);
}