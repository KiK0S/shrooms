#version 300 es
precision highp float;

out vec4 o_color;

void main() {
  float d = gl_FragCoord.z;
  o_color = vec4(d, d, d, 1.0);
}
