#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec4 out_color;

void main(void) {
  gl_Position = vec4(position, 0);
  out_color = vec4(color, 0);
}
