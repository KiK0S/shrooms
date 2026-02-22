#version 300 es
in vec3 a_pos;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
out vec2 v_world_pos;

void main() {
  vec4 world = u_model * vec4(a_pos, 1.0);
  v_world_pos = world.xy;
  gl_Position = u_proj * u_view * world;
}
