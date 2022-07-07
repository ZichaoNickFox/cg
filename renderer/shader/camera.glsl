struct Camera {
  vec3 pos_ws;
  vec3 front;
  float near;
  float far;
  mat4 view;
  mat4 project;
};
