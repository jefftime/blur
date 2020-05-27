#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec4 out_color;

// layout (binding = 0) uniform Uniforms {
//   mat4 model;
//   mat4 view;
//   mat4 projection;
// } u;

void main(void) {
  // mat4 mvp = u.projection * u.view * u.model;
  gl_Position = vec4(position, 1.0);
  out_color = vec4(color, 1);
}
