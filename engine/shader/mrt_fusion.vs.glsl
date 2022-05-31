#version 430 core

out vec2 texcoord_;

void main()
{
  const vec2 positions[4] = vec2[] (
    vec2(-1, -1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(1, 1)
  );
  const vec2 texcoords[4] = vec2[] (
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
  );
  texcoord_ = texcoords[gl_VertexID];
  gl_Position = vec4(positions[gl_VertexID], 0, 1);
}