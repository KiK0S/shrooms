#version 300 es
precision mediump float;
out vec4 o_color;

uniform sampler2D u_blur_tex;
uniform vec2 u_texel;
uniform vec2 u_direction;
uniform float u_blur_scale;

in vec2 v_uv;

void main() {
  vec2 step = u_texel * u_blur_scale;
  vec4 sum = texture(u_blur_tex, v_uv) * 0.227027;
  sum += texture(u_blur_tex, v_uv + step * u_direction * 1.384615) * 0.316216;
  sum += texture(u_blur_tex, v_uv - step * u_direction * 1.384615) * 0.316216;
  sum += texture(u_blur_tex, v_uv + step * u_direction * 3.230769) * 0.070270;
  sum += texture(u_blur_tex, v_uv - step * u_direction * 3.230769) * 0.070270;
  float mask = sum.r;
  o_color = vec4(mask, mask, mask, 1.0);
}
