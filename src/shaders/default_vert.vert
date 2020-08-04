#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform Uniforms {
  vec4 color;
} u;

vec4 mega_color = vec4(1, 0, 1, 1);

void main(void) {
  gl_Position = vec4(position, 1.0);
  // out_color = vec4(color, 1);
  out_color = u.color;

}
