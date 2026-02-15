#pragma once

#include <algorithm>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/transformation/transform_object.hpp"

#include "shrooms_screen.hpp"
#include "vfx.hpp"

namespace ambient_layers {

struct Config {
  glm::vec4 color{0.3f, 0.2f, 0.45f, 0.22f};
  float lifetime = 2.4f;
  float spawn_period = 0.55f;
  float spawn_jitter = 0.25f;
  float min_radius = 2.0f;
  float max_radius = 6.5f;
  float min_speed = -6.0f;
  float max_speed = 8.0f;
  int layer = 1;
} config;

struct BottomSporeConfig {
  glm::vec4 color{0.72f, 0.38f, 1.0f, 0.55f};
  float spawn_period = 0.18f;
  float spawn_jitter = 0.18f;
  float min_lifetime = 1.0f;
  float max_lifetime = 3.0f;
  float min_radius = 2.0f;
  float max_radius = 10.8f;
  float min_up_speed = 8.0f;
  float max_up_speed = 18.0f;
  float top_band_fraction = 0.25f;
  int layer = 0;
} bottom_config;

inline ecs::Entity* bottom_sprite_entity = nullptr;

inline void register_bottom_sprite(ecs::Entity* entity) {
  bottom_sprite_entity = entity;
}

inline bool resolve_bottom_sprite_bounds(glm::vec2& top_left, glm::vec2& size) {
  if (!bottom_sprite_entity || bottom_sprite_entity->is_pending_deletion()) return false;
  auto* transform = bottom_sprite_entity->get<transform::NoRotationTransform>();
  auto* sprite = bottom_sprite_entity->get<render_system::SpriteRenderable>();
  if (!transform || !sprite) return false;
  if (sprite->size.x <= 0.0f || sprite->size.y <= 0.0f) return false;
  top_left = transform->pos;
  size = sprite->size;
  return true;
}

struct AmbientController : public dynamic::DynamicObject {
  AmbientController() : dynamic::DynamicObject() {}
  ~AmbientController() override { Component::component_count--; }

  void update() override {
    auto* active = scene::get_active_scene();
    if (!active || active->get_name() != "main") {
      return;
    }
    if (active->is_paused_state()) return;

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    if (dt <= 0.0f) return;

    update_ambient_spores(dt);
    update_bottom_spores(dt);
  }

  void update_ambient_spores(float dt) {
    timer -= dt;
    if (timer > 0.0f) return;

    timer = config.spawn_period +
            static_cast<float>(rnd::get_double(-config.spawn_jitter, config.spawn_jitter));
    timer = std::max(0.12f, timer);

    const int count = rnd::get_int(1, 2);
    for (int i = 0; i < count; ++i) {
      const float x = static_cast<float>(rnd::get_double(0.0, shrooms::screen::view_width));
      const float y = static_cast<float>(rnd::get_double(shrooms::screen::view_height * 0.15,
                                                         shrooms::screen::view_height * 0.9));
      vfx::SporeConfig spore{};
      spore.color = config.color;
      spore.lifetime =
          static_cast<float>(rnd::get_double(config.lifetime * 0.75, config.lifetime * 1.2));
      spore.start_radius =
          static_cast<float>(rnd::get_double(config.min_radius, config.max_radius));
      spore.end_radius = spore.start_radius * 2.1f;
      spore.velocity = glm::vec2{
          static_cast<float>(rnd::get_double(config.min_speed, config.max_speed)),
          static_cast<float>(rnd::get_double(-18.0, -6.0)),
      };
      spore.layer = config.layer;
      vfx::spawn_spore(glm::vec2{x, y}, spore);
    }
  }

  void update_bottom_spores(float dt) {
    glm::vec2 floor_top_left{0.0f, 0.0f};
    glm::vec2 floor_size{0.0f, 0.0f};
    if (!resolve_bottom_sprite_bounds(floor_top_left, floor_size)) return;

    bottom_timer -= dt;
    if (bottom_timer > 0.0f) return;

    bottom_timer = bottom_config.spawn_period +
                   static_cast<float>(rnd::get_double(-bottom_config.spawn_jitter,
                                                      bottom_config.spawn_jitter));
    bottom_timer = std::max(0.12f, bottom_timer);

    const auto lifetime_range = std::minmax(bottom_config.min_lifetime, bottom_config.max_lifetime);
    const auto radius_range = std::minmax(bottom_config.min_radius, bottom_config.max_radius);
    const auto speed_range = std::minmax(bottom_config.min_up_speed, bottom_config.max_up_speed);
    const float top_band_fraction = std::clamp(bottom_config.top_band_fraction, 0.02f, 1.0f);
    const float top_band_height = std::max(1.0f, floor_size.y * top_band_fraction);

    const float x = static_cast<float>(
        rnd::get_double(floor_top_left.x, floor_top_left.x + floor_size.x));
    const float y = static_cast<float>(
        rnd::get_double(floor_top_left.y, floor_top_left.y + top_band_height));

    vfx::SporeConfig spore{};
    spore.color = bottom_config.color;
    spore.lifetime = static_cast<float>(
        rnd::get_double(lifetime_range.first, lifetime_range.second));
    spore.start_radius =
        static_cast<float>(rnd::get_double(radius_range.first, radius_range.second));
    spore.end_radius =
        spore.start_radius * static_cast<float>(rnd::get_double(1.2, 1.8));
    spore.velocity = glm::vec2{
        static_cast<float>(rnd::get_double(-2.0, 2.0)),
        -static_cast<float>(rnd::get_double(speed_range.first, speed_range.second)),
    };
    spore.layer = bottom_config.layer;
    vfx::spawn_spore(glm::vec2{x, y}, spore);
  }

  float timer = 0.0f;
  float bottom_timer = 0.0f;
};

inline void init() {
  auto* entity = arena::create<ecs::Entity>();
  entity->add(arena::create<AmbientController>());
}

}  // namespace ambient_layers
