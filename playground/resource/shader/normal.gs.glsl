#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 12) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

const float length = 0.4f;

in vec3 normal_[];
in vec3 pos_[];

void GenNormal(mat4 mvp, int index) {
  gl_Position = mvp * vec4(pos_[index], 1.0);
  EmitVertex();
  gl_Position = mvp * vec4(pos_[index] + normal_[index] * length, 1.0);
  EmitVertex();
  EndPrimitive();
}

void GenTriangle(mat4 mvp, int index0, int index1) {
  gl_Position = mvp * vec4(pos_[index0], 1.0);
  EmitVertex();
  gl_Position = mvp * vec4(pos_[index1], 1.0);
  EmitVertex();
  EndPrimitive();
}

void main()
{
  mat4 mvp = project * view * model;
  GenNormal(mvp, 0);
  GenNormal(mvp, 1);
  GenNormal(mvp, 2);
  GenTriangle(mvp, 0, 1);
  GenTriangle(mvp, 1, 2);
  GenTriangle(mvp, 0, 2);
}