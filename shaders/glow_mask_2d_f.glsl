#version 300 es
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
out vec4 o_color;
#else
precision mediump float;
#endif

uniform vec4 u_color;
uniform vec2 u_glow_center;
uniform float u_glow_radius;
uniform float u_glow_edge;
uniform float u_glow_power;
uniform float u_noise_strength;
uniform float u_noise_scale;
uniform float u_mask_strength;

in vec2 v_world_pos;

float hash(vec2 p) {
  return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main() {
  float dist = distance(v_world_pos, u_glow_center);
  float edge = smoothstep(u_glow_radius, u_glow_radius + u_glow_edge, dist);
  float mask = 1.0 - edge;
  mask = pow(clamp(mask, 0.0, 1.0), max(0.001, u_glow_power));
  float n = hash(v_world_pos * u_noise_scale);
  mask += (n - 0.5) * u_noise_strength;
  mask = clamp(mask * u_mask_strength, 0.0, 1.0);
  o_color = vec4(mask, mask, mask, 1.0) * vec4(1.0, 1.0, 1.0, u_color.a);
}
