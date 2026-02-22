#version 300 es
precision mediump float;
out vec4 o_color;

uniform sampler2D u_color_tex;
uniform sampler2D u_mask_tex;
uniform float u_divide_strength;
uniform float u_divide_epsilon;

in vec2 v_uv;

void main() {
  vec4 base = texture(u_color_tex, v_uv);
  float mask = texture(u_mask_tex, v_uv).r;
  float denom = max(1.0 - mask * u_divide_strength, u_divide_epsilon);
  vec3 rgb = clamp(base.rgb / denom, 0.0, 1.0);
  o_color = vec4(rgb, base.a);
}
