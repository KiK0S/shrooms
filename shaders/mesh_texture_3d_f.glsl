#version 300 es
precision mediump float;
out vec4 o_color;
in vec2 v_uv;
in vec4 v_color;
uniform vec4 u_color;
uniform sampler2D u_tex;

void main() {
  o_color = texture(u_tex, v_uv) * v_color * u_color;
}
