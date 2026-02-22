#version 300 es
precision mediump float;
out vec4 o_color;
in vec4 v_color;
uniform vec4 u_color;

void main() {
  o_color = v_color * u_color;
}
