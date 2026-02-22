#version 300 es
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
out vec4 o_color;
#else
precision mediump float;
#endif

uniform vec4 u_color;
uniform vec2 u_glow_center;
uniform float u_glow_inner_alpha;
uniform float u_glow_falloff;
uniform float u_glow_inner_radius;
uniform float u_glow_outer_radius;

in vec2 v_world_pos;

void main() {
  float dist = distance(v_world_pos, u_glow_center);
  float ramp = max(0.0, dist - u_glow_inner_radius);
  float alpha = u_glow_inner_alpha * exp(-u_glow_falloff * ramp);
  float edge = smoothstep(u_glow_outer_radius, u_glow_outer_radius * 1.15, dist);
  alpha *= (1.0 - edge);
  o_color = vec4(u_color.rgb, alpha * u_color.a);
}
