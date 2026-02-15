#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/moving/moving_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/rotating/rotating_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "engine/resource_ids.h"

namespace vfx {

inline float clamp01(float v) {
  return std::min(1.0f, std::max(0.0f, v));
}

inline float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

inline float ease_out(float t) {
  const float u = 1.0f - t;
  return 1.0f - u * u;
}

inline float ease_in(float t) {
  return t * t;
}

struct BurstConfig {
  std::string texture;
  float lifetime = 0.35f;
  float start_scale = 0.4f;
  float end_scale = 1.2f;
  float start_alpha = 0.9f;
  float end_alpha = 0.0f;
  float base_scale = 1.0f;
  int layer = 4;
  glm::vec4 tint{1.0f, 1.0f, 1.0f, 1.0f};
};

inline BurstConfig spawn_burst{
    "explosion",
    0.25f,
    0.2f,
    1.05f,
    0.75f,
    0.0f,
    0.85f,
    4,
    glm::vec4{0.75f, 0.45f, 1.0f, 1.0f},
};

inline BurstConfig destroy_burst{
    "explosion",
    0.45f,
    0.45f,
    1.4f,
    0.95f,
    0.0f,
    1.05f,
    4,
    glm::vec4{0.45f, 0.95f, 0.7f, 1.0f},
};

inline BurstConfig catch_burst{
    "explosion",
    0.2f,
    0.35f,
    0.95f,
    0.38f,
    0.0f,
    0.75f,
    -1,
    glm::vec4{0.75f, 0.62f, 1.0f, 1.0f},
};

inline BurstConfig miss_burst{
    "explosion",
    0.38f,
    0.7f,
    1.55f,
    0.92f,
    0.0f,
    1.05f,
    4,
    glm::vec4{1.0f, 0.28f, 0.32f, 1.0f},
};

inline BurstConfig sort_burst{
    "explosion",
    0.22f,
    0.65f,
    1.2f,
    0.9f,
    0.0f,
    0.9f,
    5,
    glm::vec4{0.9f, 0.6f, 1.0f, 1.0f},
};

inline BurstConfig projectile_burst{
    "explosion",
    0.2f,
    0.4f,
    1.0f,
    0.8f,
    0.0f,
    0.6f,
    5,
    glm::vec4{0.7f, 0.8f, 1.0f, 1.0f},
};

struct VfxBurst : public dynamic::DynamicObject {
  VfxBurst(glm::vec2 center, glm::vec2 base_size, BurstConfig config)
      : dynamic::DynamicObject(),
        center(center),
        base_size(base_size),
        config(std::move(config)) {}
  ~VfxBurst() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = config.lifetime > 0.0f ? clamp01(elapsed / config.lifetime) : 1.0f;
    const float scale = lerp(config.start_scale, config.end_scale, ease_out(t));
    const float alpha = lerp(config.start_alpha, config.end_alpha, ease_in(t));

    if (auto* sprite = entity->get<render_system::SpriteRenderable>()) {
      sprite->size = base_size * scale;
    }
    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      const glm::vec2 size = base_size * scale;
      transform->pos = center - size * 0.5f;
    }
    if (auto* tint = entity->get<color::OneColor>()) {
      tint->color.w = alpha;
    }

    if (elapsed >= config.lifetime) {
      entity->mark_deleted();
    }
  }

  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 base_size{0.0f, 0.0f};
  BurstConfig config;
  float elapsed = 0.0f;
};

struct CatchConsumeVanish : public dynamic::DynamicObject {
  CatchConsumeVanish(glm::vec2 start_center, glm::vec2 base_size, float target_x)
      : dynamic::DynamicObject(),
        start_center(start_center),
        base_size(base_size),
        target_x(target_x) {}
  ~CatchConsumeVanish() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;

    auto* sprite = entity->get<render_system::SpriteRenderable>();
    auto* transform = entity->get<transform::NoRotationTransform>();
    if (!sprite || !transform) {
      entity->mark_deleted();
      return;
    }

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = duration > 0.0f ? clamp01(elapsed / duration) : 1.0f;

    const float follow_t = ease_out(t);
    const float fade_t = ease_in(t);
    const float consumed_x = std::isfinite(target_x) ? target_x : start_center.x;
    const float wobble = std::sin(phase + t * wobble_cycles * 6.28318530718f) * base_size.x *
                         wobble_ratio * (1.0f - follow_t);
    const float y_lift = base_size.y * lift_ratio * follow_t;
    const float y_jitter =
        std::sin(phase * 1.73f + t * 18.0f) * base_size.y * 0.03f * (1.0f - follow_t);
    const glm::vec2 center{
        lerp(start_center.x, consumed_x, follow_t) + wobble,
        start_center.y - y_lift + y_jitter,
    };
    const float scale = std::max(0.04f, 1.0f - fade_t);
    const glm::vec2 scaled_size = base_size * scale;

    sprite->size = scaled_size;
    transform->pos = center - scaled_size * 0.5f;

    if (auto* tint = entity->get<color::OneColor>()) {
      tint->color.w = 1.0f - fade_t;
    }

    if (elapsed >= duration) {
      entity->mark_deleted();
    }
  }

  glm::vec2 start_center{0.0f, 0.0f};
  glm::vec2 base_size{0.0f, 0.0f};
  float target_x = std::numeric_limits<float>::quiet_NaN();
  float duration = 0.45f;
  float phase = static_cast<float>(rnd::get_double(0.0, 6.28318530718));
  float wobble_ratio = 0.09f;
  float wobble_cycles = 2.6f;
  float lift_ratio = 0.2f;
  float elapsed = 0.0f;
};

struct SporeConfig {
  glm::vec4 color{1.0f, 1.0f, 1.0f, 0.6f};
  float lifetime = 0.45f;
  float start_radius = 2.0f;
  float end_radius = 6.0f;
  glm::vec2 velocity{0.0f, 0.0f};
  int layer = 4;
};

inline SporeConfig spawn_warning_spore{
    glm::vec4{0.85f, 0.65f, 1.0f, 0.45f},
    0.35f,
    4.0f,
    18.0f,
    glm::vec2{0.0f, 0.0f},
    4,
};

struct VfxSpore : public dynamic::DynamicObject {
  VfxSpore(glm::vec2 center, SporeConfig config)
      : dynamic::DynamicObject(),
        center(center),
        config(std::move(config)) {}
  ~VfxSpore() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = config.lifetime > 0.0f ? clamp01(elapsed / config.lifetime) : 1.0f;
    const float radius = lerp(config.start_radius, config.end_radius, ease_out(t));
    center += config.velocity * dt;

    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      transform->pos = center - glm::vec2{radius, radius};
    }
    if (auto* circle = entity->get<render_system::CircleRenderable>()) {
      auto color = config.color;
      color.w = lerp(config.color.w, 0.0f, ease_in(t));
      circle->radius = radius;
      circle->color = engine::UIColor{color.x, color.y, color.z, color.w};
    }

    if (elapsed >= config.lifetime) {
      entity->mark_deleted();
    }
  }

  glm::vec2 center{0.0f, 0.0f};
  SporeConfig config;
  float elapsed = 0.0f;
};

struct SpriteShatterPiece : public dynamic::DynamicObject {
  SpriteShatterPiece(glm::vec2 center, glm::vec2 size, glm::vec2 velocity, float gravity,
                     float lifetime)
      : dynamic::DynamicObject(),
        center(center),
        size(size),
        velocity(velocity),
        gravity(gravity),
        lifetime(lifetime) {}
  ~SpriteShatterPiece() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    velocity.y += gravity * dt;
    center += velocity * dt;
    const float t = lifetime > 0.0f ? clamp01(elapsed / lifetime) : 1.0f;

    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      transform->pos = center - size * 0.5f;
    }
    if (auto* tint = entity->get<color::OneColor>()) {
      tint->color.w = 1.0f - ease_in(t);
    }

    if (elapsed >= lifetime) {
      entity->mark_deleted();
    }
  }

  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  glm::vec2 velocity{0.0f, 0.0f};
  float gravity = 0.0f;
  float lifetime = 0.3f;
  float elapsed = 0.0f;
};

struct WobbleOffset;
COMPONENT_VECTOR(WobbleOffset, wobble_offsets);

struct WobbleOffset : public dynamic::DynamicObject {
  WobbleOffset(glm::vec2 amplitude, float speed, float phase, bool respect_pause)
      : dynamic::DynamicObject(),
        amplitude(amplitude),
        speed(speed),
        phase(phase),
        respect_pause(respect_pause) {
    wobble_offsets.push_back(this);
  }
  ~WobbleOffset() override { Component::component_count--; }

  void update() override {
    if (respect_pause && scene::is_current_scene_paused()) return;
    if (!entity || entity->is_pending_deletion()) return;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    anim_time += std::max(0.0f, dt);
    const float t = anim_time;
    const glm::vec2 offset{
        std::sin(t * speed + phase) * amplitude.x,
        std::sin(t * speed * 0.73f + phase * 1.4f) * amplitude.y,
    };
    transform->translate(offset - last_offset);
    last_offset = offset;
  }

  glm::vec2 amplitude{0.0f, 0.0f};
  float speed = 1.0f;
  float phase = 0.0f;
  bool respect_pause = true;
  float anim_time = 0.0f;
  glm::vec2 last_offset{0.0f, 0.0f};
  DETACH_VECTOR(WobbleOffset, wobble_offsets)
};

inline void reset_wobble_offsets() {
  for (auto* wobble : wobble_offsets) {
    if (!wobble) continue;
    auto* entity = wobble->get_entity();
    if (!entity || entity->is_pending_deletion()) continue;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) continue;
    if (wobble->last_offset.x != 0.0f || wobble->last_offset.y != 0.0f) {
      transform->translate(glm::vec2{-wobble->last_offset.x, -wobble->last_offset.y});
    }
    wobble->last_offset = glm::vec2{0.0f, 0.0f};
    wobble->anim_time = 0.0f;
    wobble->phase = 0.0f;
  }
}

inline void attach_wobble(ecs::Entity* entity, const glm::vec2& amplitude_px, float speed,
                          bool respect_pause = true) {
  if (!entity) return;
  const float phase = static_cast<float>(rnd::get_double(0.0, 6.28318));
  entity->add(arena::create<WobbleOffset>(amplitude_px, speed, phase, respect_pause));
}

inline void spawn_spore(const glm::vec2& center, const SporeConfig& config) {
  auto* entity = arena::create<ecs::Entity>();
  const float radius = config.start_radius;
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = center - glm::vec2{radius, radius};
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(config.layer));
  entity->add(arena::create<render_system::CircleRenderable>(
      radius, engine::UIColor{config.color.x, config.color.y, config.color.z, config.color.w}));
  entity->add(arena::create<VfxSpore>(center, config));
  entity->add(arena::create<scene::SceneObject>("main"));
}

inline void spawn_spore_cloud(const glm::vec2& center, float base_radius, int count,
                              const glm::vec4& color, float spread_px, float speed_px,
                              float lifetime, int layer) {
  for (int i = 0; i < count; ++i) {
    const float ang = static_cast<float>(rnd::get_double(0.0, 6.28318));
    const float dist = static_cast<float>(rnd::get_double(0.0, spread_px));
    const glm::vec2 offset{std::cos(ang) * dist, std::sin(ang) * dist};
    const float vel = static_cast<float>(rnd::get_double(speed_px * 0.4f, speed_px));
    const glm::vec2 velocity{std::cos(ang) * vel, std::sin(ang) * vel - speed_px * 0.3f};
    SporeConfig config{};
    config.color = color;
    config.lifetime = static_cast<float>(rnd::get_double(lifetime * 0.7f, lifetime * 1.15f));
    config.start_radius = static_cast<float>(rnd::get_double(base_radius * 0.6f, base_radius));
    config.end_radius = static_cast<float>(rnd::get_double(base_radius * 1.2f, base_radius * 2.2f));
    config.velocity = velocity;
    config.layer = layer;
    spawn_spore(center + offset, config);
  }
}

inline void spawn_burst_at(const glm::vec2& center, const glm::vec2& size,
                           const BurstConfig& config) {
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 scaled_size = size * config.base_scale;
  transform->pos = center - scaled_size * 0.5f;
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(config.layer));
  const engine::TextureId tex_id = engine::resources::register_texture(config.texture);
  entity->add(arena::create<render_system::SpriteRenderable>(tex_id, scaled_size));
  glm::vec4 tint = config.tint;
  tint.w = config.start_alpha;
  entity->add(arena::create<color::OneColor>(tint));
  entity->add(arena::create<VfxBurst>(center, scaled_size, config));
  entity->add(arena::create<scene::SceneObject>("main"));
}

inline glm::vec2 entity_size(ecs::Entity* entity) {
  if (!entity) return glm::vec2{0.0f, 0.0f};
  if (auto* sprite = entity->get<render_system::SpriteRenderable>()) {
    return sprite->size;
  }
  return glm::vec2{0.0f, 0.0f};
}

inline glm::vec2 entity_center(ecs::Entity* entity, const glm::vec2& size) {
  if (!entity) return glm::vec2{0.0f, 0.0f};
  if (auto* transform = entity->get<transform::NoRotationTransform>()) {
    return transform->pos + size * 0.5f;
  }
  return glm::vec2{0.0f, 0.0f};
}

inline bool is_catch_animating(const ecs::Entity* entity) {
  return entity && entity->get<CatchConsumeVanish>();
}

inline void start_catch_consume_vanish(
    ecs::Entity* entity, float target_x = std::numeric_limits<float>::quiet_NaN()) {
  if (!entity || entity->is_pending_deletion()) return;
  if (is_catch_animating(entity)) return;

  auto* sprite = entity->get<render_system::SpriteRenderable>();
  auto* transform = entity->get<transform::NoRotationTransform>();
  if (!sprite || !transform) {
    entity->mark_deleted();
    return;
  }

  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) {
    entity->mark_deleted();
    return;
  }

  if (auto* moving = entity->get<dynamic::MovingObject>()) {
    moving->translate = glm::vec2{0.0f, 0.0f};
  }
  if (auto* rotating = entity->get<dynamic::RotatingObject>()) {
    rotating->angle = 0.0f;
  }
  if (!entity->get<color::OneColor>()) {
    entity->add(arena::create<color::OneColor>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}));
  }
  constexpr int kConsumeLayer = -1;
  if (auto* layer = entity->get<layers::ConstLayer>()) {
    layer->layer = std::min(layer->layer, kConsumeLayer);
  } else {
    entity->add(arena::create<layers::ConstLayer>(kConsumeLayer));
  }

  const glm::vec2 center = entity_center(entity, size);
  entity->add(arena::create<CatchConsumeVanish>(center, size, target_x));
}

inline void spawn_spawn_effect(const glm::vec2& center, const glm::vec2& size) {
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  spawn_burst_at(center, size, spawn_burst);
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(2.0f, extent * 0.06f);
  const float spread = extent * 0.35f;
  const float speed = std::max(30.0f, extent * 0.9f);
  spawn_spore_cloud(center, base_radius, 6, glm::vec4{0.55f, 0.9f, 0.75f, 0.55f}, spread,
                    speed, 0.5f, spawn_burst.layer + 1);
}

inline void spawn_catch_effect(
    ecs::Entity* entity, float target_x = std::numeric_limits<float>::quiet_NaN()) {
  if (!entity) return;
  if (is_catch_animating(entity)) return;
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const glm::vec2 center = entity_center(entity, size);
  start_catch_consume_vanish(entity, target_x);
  spawn_burst_at(center, size * 0.85f, catch_burst);
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(1.8f, extent * 0.045f);
  const float spread = extent * 0.25f;
  const float speed = std::max(24.0f, extent * 0.72f);
  spawn_spore_cloud(center, base_radius, 4, glm::vec4{0.78f, 0.68f, 1.0f, 0.42f}, spread, speed,
                    0.32f, catch_burst.layer);
}

inline void spawn_miss_effect(ecs::Entity* entity) {
  if (!entity) return;
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const glm::vec2 center = entity_center(entity, size);
  const float extent = std::max(size.x, size.y);
  const glm::vec2 miss_center = center + glm::vec2{0.0f, extent * 0.08f};
  spawn_burst_at(miss_center, size * 1.1f, miss_burst);

  BurstConfig shadow_burst = miss_burst;
  shadow_burst.texture = "explosion";
  shadow_burst.base_scale = 0.85f;
  shadow_burst.start_scale = 0.5f;
  shadow_burst.end_scale = 1.3f;
  shadow_burst.tint = glm::vec4{0.45f, 0.05f, 0.1f, 1.0f};
  shadow_burst.layer = miss_burst.layer - 1;
  spawn_burst_at(miss_center + glm::vec2{0.0f, extent * 0.05f}, size, shadow_burst);

  const float base_radius = std::max(2.0f, extent * 0.05f);
  const float spread = extent * 0.32f;
  const float speed = std::max(38.0f, extent * 1.0f);
  spawn_spore_cloud(miss_center + glm::vec2{0.0f, extent * 0.05f}, base_radius, 7,
                    glm::vec4{0.9f, 0.2f, 0.28f, 0.65f}, spread, speed, 0.55f,
                    miss_burst.layer + 1);

  for (int i = 0; i < 4; ++i) {
    SporeConfig trail{};
    trail.color = glm::vec4{0.7f, 0.1f, 0.2f, 0.55f};
    trail.lifetime = static_cast<float>(rnd::get_double(0.28, 0.42));
    trail.start_radius = static_cast<float>(rnd::get_double(base_radius * 0.65f, base_radius));
    trail.end_radius =
        static_cast<float>(rnd::get_double(base_radius * 1.35f, base_radius * 2.1f));
    trail.velocity = glm::vec2{
        static_cast<float>(rnd::get_double(-20.0, 20.0)),
        static_cast<float>(rnd::get_double(45.0, 120.0)),
    };
    trail.layer = miss_burst.layer + 1;
    const glm::vec2 offset{
        static_cast<float>(rnd::get_double(-spread * 0.35f, spread * 0.35f)),
        static_cast<float>(rnd::get_double(-spread * 0.15f, spread * 0.25f)),
    };
    spawn_spore(miss_center + offset, trail);
  }
}

inline void spawn_sort_effect(ecs::Entity* entity) {
  if (!entity) return;
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const glm::vec2 center = entity_center(entity, size);
  spawn_burst_at(center, size, sort_burst);
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(2.0f, extent * 0.05f);
  const float spread = extent * 0.25f;
  const float speed = std::max(35.0f, extent * 0.9f);
  spawn_spore_cloud(center, base_radius, 7, glm::vec4{0.85f, 0.65f, 1.0f, 0.6f}, spread, speed,
                    0.4f, sort_burst.layer + 1);
}

inline void spawn_sprite_shatter(ecs::Entity* entity, int columns = 3, int rows = 3) {
  if (!entity) return;
  auto* sprite = entity->get<render_system::SpriteRenderable>();
  auto* transform = entity->get<transform::NoRotationTransform>();
  if (!sprite || !transform) return;
  if (sprite->size.x <= 0.0f || sprite->size.y <= 0.0f) return;

  columns = std::max(1, columns);
  rows = std::max(1, rows);
  const glm::vec2 piece_size{
      sprite->size.x / static_cast<float>(columns),
      sprite->size.y / static_cast<float>(rows),
  };
  if (piece_size.x <= 0.0f || piece_size.y <= 0.0f) return;

  int base_layer = sort_burst.layer + 1;
  if (auto* layered = entity->get<layers::LayeredObject>()) {
    base_layer = layered->get_layer() + 1;
  }

  const glm::vec2 center = transform->pos + sprite->size * 0.5f;
  const float extent = std::max(sprite->size.x, sprite->size.y);
  const float min_speed = std::max(90.0f, extent * 1.6f);
  const float max_speed = std::max(130.0f, extent * 2.35f);

  for (int y = 0; y < rows; ++y) {
    for (int x = 0; x < columns; ++x) {
      auto* piece = arena::create<ecs::Entity>();

      const glm::vec2 top_left = transform->pos +
                                 glm::vec2{piece_size.x * static_cast<float>(x),
                                           piece_size.y * static_cast<float>(y)};
      const glm::vec2 piece_center = top_left + piece_size * 0.5f;

      auto* piece_transform = arena::create<transform::NoRotationTransform>();
      piece_transform->pos = top_left;
      piece->add(piece_transform);
      piece->add(arena::create<layers::ConstLayer>(base_layer));

      auto* piece_sprite =
          arena::create<render_system::SpriteRenderable>(sprite->texture_id, piece_size);
      const float u0 = static_cast<float>(x) / static_cast<float>(columns);
      const float v0 = static_cast<float>(y) / static_cast<float>(rows);
      const float u1 = static_cast<float>(x + 1) / static_cast<float>(columns);
      const float v1 = static_cast<float>(y + 1) / static_cast<float>(rows);
      const std::array<float, 8> uvs{u0, v0, u1, v0, u0, v1, u1, v1};
      piece_sprite->geometry = engine::geometry::make_quad(piece_size.x, piece_size.y, uvs);
      piece->add(piece_sprite);
      piece->add(arena::create<color::OneColor>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}));

      glm::vec2 direction = piece_center - center;
      if (glm::length(direction) <= 0.0001f) {
        direction = glm::vec2{
            static_cast<float>(rnd::get_double(-0.8, 0.8)),
            static_cast<float>(rnd::get_double(-1.0, 0.4)),
        };
      }
      direction.x += static_cast<float>(rnd::get_double(-0.28, 0.28));
      direction.y += static_cast<float>(rnd::get_double(-0.24, 0.24));
      if (glm::length(direction) <= 0.0001f) {
        direction = glm::vec2{0.0f, -1.0f};
      } else {
        direction = glm::normalize(direction);
      }

      const float speed = static_cast<float>(rnd::get_double(min_speed, max_speed));
      const glm::vec2 velocity = direction * speed + glm::vec2{0.0f, -speed * 0.18f};
      const float lifetime = static_cast<float>(rnd::get_double(0.24, 0.38));
      piece->add(arena::create<SpriteShatterPiece>(piece_center, piece_size, velocity, 540.0f,
                                                   lifetime));
      piece->add(arena::create<scene::SceneObject>("main"));
    }
  }
}

inline void spawn_projectile_flash(const glm::vec2& center, const glm::vec2& size) {
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  spawn_burst_at(center, size, projectile_burst);
}

inline void spawn_projectile_trail(const glm::vec2& center, float radius) {
  SporeConfig config{};
  config.color = glm::vec4{0.7f, 0.6f, 1.0f, 0.45f};
  config.lifetime = 0.18f;
  config.start_radius = std::max(1.5f, radius * 0.45f);
  config.end_radius = std::max(2.5f, radius * 0.85f);
  config.velocity = glm::vec2{0.0f, 20.0f};
  config.layer = projectile_burst.layer + 1;
  spawn_spore(center, config);
}

inline void spawn_spawn_warning(const glm::vec2& center, const glm::vec2& size, float lifetime) {
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  SporeConfig config = spawn_warning_spore;
  config.lifetime = std::max(0.12f, lifetime);
  const float extent = std::max(size.x, size.y);
  config.start_radius = std::max(2.0f, extent * 0.18f);
  config.end_radius = std::max(config.start_radius + 2.0f, extent * 0.6f);
  spawn_spore(center, config);
}

inline void spawn_destroy_effect(ecs::Entity* entity) {
  if (!entity) return;
  spawn_sort_effect(entity);
  spawn_sprite_shatter(entity, 3, 3);
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const glm::vec2 center = entity_center(entity, size);
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(2.0f, extent * 0.045f);
  const float spread = extent * 0.28f;
  const float speed = std::max(25.0f, extent * 0.82f);
  spawn_spore_cloud(center, base_radius, 6, glm::vec4{0.82f, 0.76f, 0.95f, 0.6f}, spread, speed,
                    0.34f, sort_burst.layer + 2);
}

}  // namespace vfx
