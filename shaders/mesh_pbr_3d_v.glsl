#version 300 es
precision highp float;

in vec3 a_pos;
in vec3 a_normal;
in vec2 a_uv;
in vec4 a_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform mat4 u_light_view_proj;

out vec3 v_world_pos;
out vec3 v_world_normal;
out vec2 v_uv;
out vec4 v_color;
out vec4 v_shadow_pos;

void main() {
  vec4 world = u_model * vec4(a_pos, 1.0);
  v_world_pos = world.xyz;
  mat3 normal_mat = mat3(transpose(inverse(u_model)));
  v_world_normal = normalize(normal_mat * a_normal);
  v_uv = a_uv;
  v_color = a_color;
  v_shadow_pos = u_light_view_proj * world;
  gl_Position = u_proj * u_view * world;
}
