#version 300 es
precision highp float;

in vec3 v_world_pos;
in vec3 v_world_normal;
in vec2 v_uv;
in vec4 v_color;
in vec4 v_shadow_pos;

uniform vec3 u_camera_pos;
uniform vec4 u_base_color_factor;
uniform float u_metallic_factor;
uniform float u_roughness_factor;
uniform vec3 u_emissive_factor;
uniform float u_normal_scale;
uniform float u_occlusion_strength;

uniform float u_has_base_color_tex;
uniform float u_has_metal_rough_tex;
uniform float u_has_normal_tex;
uniform float u_has_occlusion_tex;
uniform float u_has_emissive_tex;

uniform sampler2D u_base_color_tex;
uniform sampler2D u_metal_rough_tex;
uniform sampler2D u_normal_tex;
uniform sampler2D u_occlusion_tex;
uniform sampler2D u_emissive_tex;
uniform sampler2D u_shadow_map;

uniform float u_enable_shadows;
uniform float u_enable_ao;
uniform float u_enable_normal_map;
uniform float u_shadow_bias;

uniform float u_light_count;
uniform vec3 u_light_pos[8];
uniform vec3 u_light_dir[8];
uniform vec3 u_light_color[8];
uniform vec4 u_light_params[8]; // x=type, y=range, z=innerCos, w=outerCos

uniform vec3 u_ambient;

out vec4 o_color;

const float PI = 3.14159265359;

vec3 srgb_to_linear(vec3 c) {
  return pow(c, vec3(2.2));
}

vec3 linear_to_srgb(vec3 c) {
  return pow(c, vec3(1.0 / 2.2));
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  return a2 / (PI * denom * denom + 0.000001);
}

float geometry_schlick_ggx(float NdotV, float roughness) {
  float r = roughness + 1.0;
  float k = (r * r) / 8.0;
  float denom = NdotV * (1.0 - k) + k;
  return NdotV / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = geometry_schlick_ggx(NdotV, roughness);
  float ggx1 = geometry_schlick_ggx(NdotL, roughness);
  return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 get_normal() {
  vec3 N = normalize(v_world_normal);
  if (u_enable_normal_map < 0.5 || u_has_normal_tex < 0.5) {
    return N;
  }
  vec3 tangent_normal = texture(u_normal_tex, v_uv).xyz * 2.0 - 1.0;
  tangent_normal.xy *= u_normal_scale;
  vec3 Q1 = dFdx(v_world_pos);
  vec3 Q2 = dFdy(v_world_pos);
  vec2 st1 = dFdx(v_uv);
  vec2 st2 = dFdy(v_uv);
  vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
  vec3 B = normalize(-Q1 * st2.s + Q2 * st1.s);
  mat3 TBN = mat3(T, B, N);
  return normalize(TBN * tangent_normal);
}

void main() {
  vec4 base = u_base_color_factor * v_color;
  if (u_has_base_color_tex > 0.5) {
    base *= texture(u_base_color_tex, v_uv);
  }
  float alpha = base.a;
  vec3 albedo = srgb_to_linear(base.rgb);

  float metallic = u_metallic_factor;
  float roughness = u_roughness_factor;
  if (u_has_metal_rough_tex > 0.5) {
    vec4 mr = texture(u_metal_rough_tex, v_uv);
    roughness *= mr.g;
    metallic *= mr.b;
  }
  roughness = clamp(roughness, 0.04, 1.0);

  vec3 emissive = u_emissive_factor;
  if (u_has_emissive_tex > 0.5) {
    emissive *= srgb_to_linear(texture(u_emissive_tex, v_uv).rgb);
  }

  float ao = 1.0;
  if (u_enable_ao > 0.5 && u_has_occlusion_tex > 0.5) {
    ao = mix(1.0, texture(u_occlusion_tex, v_uv).r, u_occlusion_strength);
  }

  vec3 N = get_normal();
  vec3 V = normalize(u_camera_pos - v_world_pos);
  vec3 F0 = mix(vec3(0.04), albedo, metallic);

  vec3 Lo = vec3(0.0);
  int light_count = int(u_light_count + 0.5);
  for (int i = 0; i < 8; ++i) {
    if (i >= light_count) break;
    float type = u_light_params[i].x;
    float range = u_light_params[i].y;
    float inner_cos = u_light_params[i].z;
    float outer_cos = u_light_params[i].w;

    vec3 L;
    float attenuation = 1.0;
    if (type < 0.5) {
      L = normalize(-u_light_dir[i]);
    } else {
      vec3 to_light = u_light_pos[i] - v_world_pos;
      float dist = length(to_light);
      L = dist > 0.0 ? to_light / dist : vec3(0.0, 0.0, 1.0);
      attenuation = 1.0 / max(dist * dist, 0.01);
      if (range > 0.0) {
        float falloff = clamp(1.0 - dist / range, 0.0, 1.0);
        attenuation *= falloff * falloff;
      }
      if (type > 1.5) {
        vec3 light_dir = normalize(u_light_dir[i]);
        float cos_theta = dot(light_dir, normalize(v_world_pos - u_light_pos[i]));
        float spot = smoothstep(outer_cos, inner_cos, cos_theta);
        attenuation *= spot;
      }
    }

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) continue;

    vec3 H = normalize(V + L);
    float NDF = distribution_ggx(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
    vec3 specular = numerator / denom;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 radiance = u_light_color[i] * attenuation;
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  vec3 color = emissive + Lo + u_ambient * albedo * ao;

  if (u_enable_shadows > 0.5) {
    vec3 shadow_ndc = v_shadow_pos.xyz / v_shadow_pos.w;
    vec2 shadow_uv = shadow_ndc.xy * 0.5 + 0.5;
    float shadow_depth = shadow_ndc.z * 0.5 + 0.5;
    if (shadow_uv.x >= 0.0 && shadow_uv.x <= 1.0 && shadow_uv.y >= 0.0 && shadow_uv.y <= 1.0) {
      float map_depth = texture(u_shadow_map, shadow_uv).r;
      float shadow = shadow_depth - u_shadow_bias > map_depth ? 0.35 : 1.0;
      color *= shadow;
    }
  }
  color = linear_to_srgb(color);
  o_color = vec4(color, alpha);
}
