#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <vector>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/animation/animation_system.hpp"
#include "systems/animation/sprite_animation.hpp"
#include "systems/collision/collider_object.hpp"
#include "systems/geometry/shapes/quad.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/render/implicit_skeletoned_sprite.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/moving/moving_object.hpp"
#include "engine/resource_ids.h"
#include "ecs/context.hpp"
#include "systems/scene/scene_system.hpp"

#include "level_manager.hpp"
#include "vfx.hpp"
#include "camera_shake.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "touchscreen.hpp"

namespace player {

inline ecs::Entity* player_entity = nullptr;
inline transform::NoRotationTransform* player_transform = nullptr;
inline glm::vec2 player_size{0.0f, 0.0f};
inline animation::SpriteAnimation* player_anim = nullptr;

struct PlayerVibe : public dynamic::DynamicObject {
  PlayerVibe() : dynamic::DynamicObject() {}
  ~PlayerVibe() override { Component::component_count--; }

  void kick(float amount) {
    recoil = std::min(1.0f, recoil + amount);
  }

  void update() override {
    if (scene::is_current_scene_paused()) return;
    if (!player_transform) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    const float t = static_cast<float>(ecs::context().time_seconds);
    recoil = std::max(0.0f, recoil - recoil_decay * dt);

    const float bob = std::sin(t * bob_speed + phase) * bob_amp_px;
    const float sway = std::sin(t * bob_speed * 0.6f + phase * 1.3f) * sway_amp_px;
    const float recoil_y = -recoil * bob_amp_px * 2.2f;
    const glm::vec2 offset{sway, bob + recoil_y};
    player_transform->translate(offset - last_offset);
    last_offset = offset;
  }

  float bob_amp_px = 0.0f;
  float sway_amp_px = 0.0f;
  float bob_speed = 3.2f;
  float recoil_decay = 7.5f;
  float recoil = 0.0f;
  float phase = 0.0f;
  glm::vec2 last_offset{0.0f, 0.0f};
};

inline PlayerVibe* player_vibe = nullptr;
inline struct PlayerController* player_controller = nullptr;

struct CarriedMarker : public ecs::Component {
  explicit CarriedMarker(ecs::Entity* carrier) : ecs::Component(), carrier(carrier) {}
  ~CarriedMarker() override { Component::component_count--; }

  ecs::Entity* carrier = nullptr;
};

inline void kick_recoil(float amount) {
  if (!player_vibe) return;
  player_vibe->kick(amount);
}

enum class FamiliarState {
  Orbit,
  Planted,
  Carry,
  StrikeAlign,
  StrikeAscend,
  Returning,
};

struct FamiliarLogic : public dynamic::DynamicObject {
  FamiliarLogic(float orbit_radius_px, float orbit_speed, float phase)
      : dynamic::DynamicObject(),
        orbit_radius_px(orbit_radius_px),
        orbit_speed(orbit_speed),
        orbit_angle(phase),
        orbit_phase(phase) {}
  ~FamiliarLogic() override { Component::component_count--; }

  void update() override {
    if (scene::is_current_scene_paused()) return;
    if (!player_transform) return;

    if (!transform) {
      transform = entity ? entity->get<transform::NoRotationTransform>() : nullptr;
      if (!transform) return;
    }

    const float dt = static_cast<float>(ecs::context().delta_seconds);

    if (carried && carried->is_pending_deletion()) {
      clear_carried(false);
      begin_return();
    }

    const glm::vec2 player_center =
        player_transform->pos + glm::vec2{player_size.x * 0.5f, player_size.y * 0.5f};

    if (state == FamiliarState::Orbit) {
      orbit_angle += orbit_speed * dt;
      const glm::vec2 offset{
          std::cos(orbit_angle) * orbit_radius_px,
          std::sin(orbit_angle) * orbit_radius_px * orbit_y_scale,
      };
      set_center(player_center + offset);
    } else if (state == FamiliarState::Planted) {
      set_center(planted_center);
    } else if (state == FamiliarState::Carry) {
      glm::vec2 center = current_center();
      const glm::vec2 to_player = player_center - center;
      const float dist = glm::length(to_player);
      if (dist <= carry_speed * dt + 1.0f) {
        set_center(player_center + carry_offset);
        deliver();
      } else if (dist > 0.0001f) {
        center += glm::normalize(to_player) * carry_speed * dt;
        set_center(center);
      }
    } else if (state == FamiliarState::StrikeAlign) {
      glm::vec2 center = current_center();
      const float dx = strike_lane_x - center.x;
      const float step = strike_align_speed * dt;
      if (std::abs(dx) <= step) {
        center.x = strike_lane_x;
        set_center(center);
        state = FamiliarState::StrikeAscend;
      } else if (dx > 0.0f) {
        center.x += step;
        set_center(center);
      } else {
        center.x -= step;
        set_center(center);
      }
    } else if (state == FamiliarState::StrikeAscend) {
      glm::vec2 center = current_center();
      center.x += (strike_lane_x - center.x) * std::min(1.0f, strike_lane_blend * dt);
      center.y -= strike_up_speed * dt;
      set_center(center);
      if (center.y <= strike_top_y) {
        begin_return(strike_return_delay);
      }
    } else if (state == FamiliarState::Returning) {
      return_timer = std::max(0.0f, return_timer - dt);
      glm::vec2 center = current_center();
      const glm::vec2 to_player = player_center - center;
      const float dist = glm::length(to_player);
      bool reached_player = false;
      if (dist > 1.0f && dist > 0.0001f) {
        center += glm::normalize(to_player) * return_speed * dt;
        set_center(center);
      } else {
        set_center(player_center);
        reached_player = true;
      }
      if (reached_player && return_timer <= 0.0f) {
        state = FamiliarState::Orbit;
      }
    }

    update_carried_position();

    if (state == FamiliarState::Carry || state == FamiliarState::StrikeAscend) {
      trail_timer -= dt;
      if (trail_timer <= 0.0f) {
        vfx::spawn_projectile_trail(current_center(), size.x * 0.25f);
        trail_timer = trail_period;
      }
    }
  }

  bool is_idle() const { return state == FamiliarState::Orbit; }

  bool can_capture() const { return state == FamiliarState::Planted && !carried; }

  bool can_strike_hit() const {
    return state == FamiliarState::StrikeAlign || state == FamiliarState::StrikeAscend;
  }

  void launch_strike() {
    if (!is_idle()) return;
    if (player_transform) {
      const glm::vec2 player_center =
          player_transform->pos + glm::vec2{player_size.x * 0.5f, player_size.y * 0.5f};
      strike_lane_x = player_center.x;
    } else {
      strike_lane_x = current_center().x;
    }
    strike_top_y = -size.y * 0.6f;
    state = FamiliarState::StrikeAscend;
    trail_timer = 0.0f;
    kick_recoil(0.45f);
    vfx::spawn_projectile_flash(current_center(), size);
  }

  void handle_strike_hit(ecs::Entity* mushroom) {
    if (!mushroom || mushroom->is_pending_deletion()) return;
    if (vfx::is_catch_animating(mushroom)) return;
    if (!can_strike_hit()) return;
    levels::on_mushroom_sorted(mushroom);
    begin_return(strike_return_delay);
  }

  void deploy() {
    if (!is_idle()) return;
    if (player_transform) {
      const glm::vec2 player_center =
          player_transform->pos + glm::vec2{player_size.x * 0.5f, player_size.y * 0.5f};
      planted_center = player_center + glm::vec2{0.0f, -player_size.y * 0.65f};
    } else {
      planted_center = current_center();
    }
    state = FamiliarState::Planted;
  }

  void handle_capture(ecs::Entity* mushroom) {
    if (!mushroom || mushroom->is_pending_deletion()) return;
    if (vfx::is_catch_animating(mushroom)) return;
    if (!can_capture()) return;
    if (mushroom->get<CarriedMarker>()) return;

    mushroom->add(arena::create<CarriedMarker>(entity));
    carried = mushroom;
    carried_transform = mushroom->get<transform::NoRotationTransform>();
    carried_size = vfx::entity_size(mushroom);
    if (carried_size.x <= 0.0f || carried_size.y <= 0.0f) {
      carried_size = size * 0.8f;
    }

    if (auto* moving = mushroom->get<dynamic::MovingObject>()) {
      moving->translate = glm::vec2{0.0f, 0.0f};
    }

    carry_offset = glm::vec2{0.0f, -player_size.y * 0.15f};
    state = FamiliarState::Carry;
    trail_timer = 0.0f;
    vfx::spawn_projectile_flash(current_center(), size);
  }

  void reset() {
    clear_carried(true);
    state = FamiliarState::Orbit;
    return_timer = 0.0f;
    trail_timer = 0.0f;
    orbit_angle = orbit_phase;
    if (!transform) {
      transform = entity ? entity->get<transform::NoRotationTransform>() : nullptr;
    }
    if (transform && player_transform) {
      const glm::vec2 player_center =
          player_transform->pos + glm::vec2{player_size.x * 0.5f, player_size.y * 0.5f};
      const glm::vec2 offset{
          std::cos(orbit_angle) * orbit_radius_px,
          std::sin(orbit_angle) * orbit_radius_px * orbit_y_scale,
      };
      set_center(player_center + offset);
      strike_lane_x = player_center.x;
    }
    planted_center = current_center();
    strike_top_y = -size.y * 0.6f;
  }

  void clear_carried(bool delete_entity) {
    if (!carried) return;
    if (delete_entity) {
      carried->mark_deleted();
    }
    carried = nullptr;
    carried_transform = nullptr;
    carried_size = glm::vec2{0.0f, 0.0f};
  }

  void deliver() {
    if (!carried || carried->is_pending_deletion()) {
      clear_carried(false);
      begin_return();
      return;
    }

    float catch_x = std::numeric_limits<float>::quiet_NaN();
    if (player_transform) {
      const glm::vec2 player_center =
          player_transform->pos + glm::vec2{player_size.x * 0.5f, player_size.y * 0.5f};
      catch_x = player_center.x;
      if (carried_transform) {
        carried_transform->pos = shrooms::screen::center_to_top_left(player_center, carried_size);
      }
    }

    auto* sprite = carried->get<render_system::SpriteRenderable>();
    const std::string type = sprite ? engine::resources::texture_name(sprite->texture_id) : "";
    levels::on_mushroom_caught(type, carried, catch_x, true);
    clear_carried(false);
    begin_return();
  }

  void begin_return(float delay = -1.0f) {
    state = FamiliarState::Returning;
    return_timer = delay >= 0.0f ? delay : return_delay;
  }

  glm::vec2 current_center() const {
    if (!transform) return glm::vec2{0.0f, 0.0f};
    return transform->pos + size * 0.5f;
  }

  void set_center(const glm::vec2& center) {
    if (!transform) return;
    transform->pos = center - size * 0.5f;
  }

  void update_carried_position() {
    if (!carried || !carried_transform) return;
    const glm::vec2 target_center =
        current_center() + glm::vec2{0.0f, size.y * 0.35f};
    carried_transform->pos = target_center - carried_size * 0.5f;
  }

  transform::NoRotationTransform* transform = nullptr;
  glm::vec2 size{0.0f, 0.0f};
  float orbit_radius_px = 0.0f;
  float orbit_speed = 2.3f;
  float orbit_angle = 0.0f;
  float orbit_phase = 0.0f;
  float orbit_y_scale = 0.65f;
  float carry_speed = 820.0f;
  float strike_align_speed = 880.0f;
  float strike_up_speed = 980.0f;
  float strike_lane_blend = 8.0f;
  float strike_top_y = -50.0f;
  float strike_lane_x = 0.0f;
  float strike_return_delay = 0.22f;
  float return_speed = 720.0f;
  float return_delay = 1.0f;
  float return_timer = 0.0f;
  float trail_timer = 0.0f;
  float trail_period = 0.05f;
  glm::vec2 carry_offset{0.0f, 0.0f};
  glm::vec2 planted_center{0.0f, 0.0f};
  FamiliarState state = FamiliarState::Orbit;
  ecs::Entity* carried = nullptr;
  transform::NoRotationTransform* carried_transform = nullptr;
  glm::vec2 carried_size{0.0f, 0.0f};
};

inline constexpr int kFamiliarCount = 3;
inline std::array<ecs::Entity*, kFamiliarCount> familiar_entities{};
inline std::array<FamiliarLogic*, kFamiliarCount> familiar_logic{};
inline glm::vec2 familiar_size{0.0f, 0.0f};

inline FamiliarLogic* find_idle_familiar() {
  for (auto* logic : familiar_logic) {
    if (!logic) continue;
    if (logic->is_idle()) return logic;
  }
  return nullptr;
}

inline FamiliarLogic* find_idle_familiar_for_strike() {
  if (!player_transform) return find_idle_familiar();
  FamiliarLogic* best = nullptr;
  const float lane_x = player_transform->pos.x + player_size.x * 0.5f;
  float best_distance = 0.0f;
  for (auto* logic : familiar_logic) {
    if (!logic || !logic->is_idle()) continue;
    const float distance = std::abs(logic->current_center().x - lane_x);
    if (!best || distance < best_distance) {
      best = logic;
      best_distance = distance;
    }
  }
  return best;
}

inline void deploy_familiar() {
  if (auto* logic = find_idle_familiar()) {
    logic->deploy();
  }
}

inline void launch_strike_familiar() {
  if (auto* logic = find_idle_familiar_for_strike()) {
    logic->launch_strike();
  }
}

inline collision::TriggerObject* make_familiar_trigger(FamiliarLogic* logic) {
  return arena::create<collision::TriggerObject>(
      "mushroom_catch_handler",
      [logic](ecs::Entity*, collision::ColliderObject* collider) {
        if (!logic || !collider) return;
        if (!logic->can_capture()) return;
        auto* entity = collider->get_entity();
        if (!entity || entity->is_pending_deletion()) return;
        if (vfx::is_catch_animating(entity)) return;
        logic->handle_capture(entity);
      });
}

inline collision::TriggerObject* make_familiar_sort_trigger(FamiliarLogic* logic) {
  return arena::create<collision::TriggerObject>(
      "bone_projectile_handler",
      [logic](ecs::Entity*, collision::ColliderObject* collider) {
        if (!logic || !collider) return;
        auto* entity = collider->get_entity();
        if (!entity || entity->is_pending_deletion()) return;
        if (vfx::is_catch_animating(entity)) return;
        logic->handle_strike_hit(entity);
      });
}

inline void init_familiars() {
  if (!player_transform) return;
  familiar_size = shrooms::texture_sizing::from_reference_width("famiriar", 29.0f);
  const float orbit_radius = player_size.x * 2.0f;
  const float orbit_speed = 2.4f;
  const engine::TextureId tex_id = engine::resources::register_texture("famiriar");

  for (int i = 0; i < kFamiliarCount; ++i) {
    auto* entity = arena::create<ecs::Entity>();
    auto* transform = arena::create<transform::NoRotationTransform>();
    const float phase = static_cast<float>(i) * 6.28318f / static_cast<float>(kFamiliarCount);
    const glm::vec2 player_center =
        player_transform->pos + glm::vec2{player_size.x * 0.5f, player_size.y * 0.5f};
    const glm::vec2 offset{
        std::cos(phase) * orbit_radius,
        std::sin(phase) * orbit_radius * 0.65f,
    };
    transform->pos = shrooms::screen::center_to_top_left(player_center + offset, familiar_size);
    entity->add(transform);

    entity->add(arena::create<geometry::Quad>(
        "familiar_collider",
        std::vector<glm::vec2>{
            glm::vec2{0.0f, 0.0f},
            glm::vec2{familiar_size.x, 0.0f},
            glm::vec2{0.0f, familiar_size.y},
            glm::vec2{familiar_size.x, familiar_size.y},
        }));

    entity->add(arena::create<layers::ConstLayer>(3));
    auto* familiar_sprite =
        arena::create<render_system::ImplicitSkeletonedSprite>(tex_id, familiar_size);
    familiar_sprite->warp_power = 2.0f;
    familiar_sprite->warp_epsilon = 0.02f;
    familiar_sprite->warp_rest_weight = 1.25f;
    familiar_sprite->point_generator =
        [](float time_seconds, std::vector<render_system::ImplicitWarpPoint>& out_points) {
          out_points.clear();
          out_points.reserve(9);

          auto add_point = [&](float from_x, float from_y, float to_x, float to_y,
                               float radius_uv) {
            const float clamped_from_x = std::clamp(from_x, 0.0f, 1.0f);
            const float clamped_from_y = std::clamp(from_y, 0.0f, 1.0f);
            const float clamped_to_x = std::clamp(to_x, 0.0f, 1.0f);
            const float clamped_to_y = std::clamp(to_y, 0.0f, 1.0f);
            out_points.push_back(render_system::ImplicitWarpPoint{
                engine::Vec2{clamped_from_x, clamped_from_y},
                engine::Vec2{clamped_to_x, clamped_to_y},
                std::max(0.0f, radius_uv),
            });
          };

          // Keep sprite corners pinned to avoid border drift.
          add_point(0.0f, 0.0f, 0.0f, 0.0f, 0.65f);
          add_point(1.0f, 0.0f, 1.0f, 0.0f, 0.65f);
          add_point(0.0f, 1.0f, 0.0f, 1.0f, 0.65f);
          add_point(1.0f, 1.0f, 1.0f, 1.0f, 0.65f);

          // Hold near extremes and snap between them for a flicker-like motion.
          auto flicker_wave = [](float t) {
            const float hold = 0.38f;
            const float travel = 0.12f;
            const float cycle = hold + travel + hold + travel;
            float p = std::fmod(t, cycle);
            if (p < 0.0f) p += cycle;

            auto smooth01 = [](float u) {
              u = std::clamp(u, 0.0f, 1.0f);
              return u * u * (3.0f - 2.0f * u);
            };

            if (p < hold) return 1.0f;
            p -= hold;
            if (p < travel) {
              const float u = smooth01(p / travel);
              return 1.0f - 2.0f * u;
            }
            p -= travel;
            if (p < hold) return -1.0f;
            p -= hold;
            const float u = smooth01(p / travel);
            return -1.0f + 2.0f * u;
          };

          const float phase = time_seconds * 4.0f;
          const float wave = flicker_wave(phase);

          // Joint layout authored in texture-space pixels.
          constexpr float kTexWidth = 28.5f;
          constexpr float kTexHeight = 14.25f;
          auto to_uv_x = [](float px) { return px / kTexWidth; };
          auto to_uv_y = [](float py) { return py / kTexHeight; };

          const float left_inner_x_base = to_uv_x(10.0f);
          const float left_inner_y_base = to_uv_y(4.0f);
          const float right_inner_x_base = to_uv_x(21.0f);
          const float right_inner_y_base = to_uv_y(2.0f);
          const float left_outer_x_base = to_uv_x(0.0f);
          const float left_outer_y_base = to_uv_y(11.0f);
          const float right_outer_x_base = to_uv_x(28.5f);
          const float right_outer_y_base = to_uv_y(9.0f);

          // Center joint: midpoint of inner joints.
          const float center_x = 0.5f * (left_inner_x_base + right_inner_x_base);
          const float center_y_base = 0.5f * (left_inner_y_base + right_inner_y_base);
          const float center_y_delta = 0.07f * wave;
          const float center_y = center_y_base + center_y_delta;

          const float inner_x_delta = 0.035f * wave;
          const float left_inner_x = left_inner_x_base + inner_x_delta;
          const float right_inner_x = right_inner_x_base - inner_x_delta;

          // Give outer joints a slightly stronger swing for more expression.
          const float outer_x_delta = 0.052f * wave;
          const float left_outer_x = left_inner_x_base + outer_x_delta;
          const float right_outer_x = right_inner_x_base - outer_x_delta;

          add_point(left_outer_x_base, left_outer_y_base, left_outer_x, center_y, 0.42f);
          add_point(left_inner_x_base, left_inner_y_base, left_inner_x, center_y, 0.34f);
          add_point(center_x, center_y_base, center_x, center_y, 0.30f);
          add_point(right_inner_x_base, right_inner_y_base, right_inner_x, center_y, 0.34f);
          add_point(right_outer_x_base, right_outer_y_base, right_outer_x, center_y, 0.42f);
        };
    entity->add(familiar_sprite);
    auto* logic = arena::create<FamiliarLogic>(orbit_radius, orbit_speed, phase);
    logic->size = familiar_size;
    entity->add(logic);
    entity->add(make_familiar_trigger(logic));
    entity->add(make_familiar_sort_trigger(logic));
    entity->add(arena::create<scene::SceneObject>("main"));

    familiar_entities[i] = entity;
    familiar_logic[i] = logic;
  }
}

inline void reset_familiars() {
  for (auto* logic : familiar_logic) {
    if (!logic) continue;
    logic->reset();
  }
}

struct PlayerController : public dynamic::DynamicObject {
  explicit PlayerController(float step_px) : dynamic::DynamicObject(), step_px(step_px) {}
  ~PlayerController() override { Component::component_count--; }

  void update() override {
    if (scene::is_current_scene_paused()) return;
    if (!player_transform) return;

    const bool deploy_pressed =
        input::get_button_state('E') || touchscreen::deploy_pressed;
    if (deploy_pressed && !deploy_pressed_last) {
      deploy_familiar();
    }
    deploy_pressed_last = deploy_pressed;

    const bool fire_pressed =
        input::is_down(input::Key::W) || touchscreen::fire_pressed;
    if (fire_pressed && !fire_pressed_last) {
      launch_strike_familiar();
    }
    fire_pressed_last = fire_pressed;

    float dx = 0.0f;
    if (input::is_down(input::Key::A)) dx -= step_px;
    if (input::is_down(input::Key::D)) dx += step_px;
    dx += touchscreen::joystick_value.x * step_px;

    if (dx == 0.0f) {
      dust_timer = 0.0f;
      return;
    }

    player_transform->pos.x += dx;
    const float min_x = 0.0f;
    const float max_x = static_cast<float>(shrooms::screen::view_width) - player_size.x;
    if (player_transform->pos.x < min_x) player_transform->pos.x = min_x;
    if (player_transform->pos.x > max_x) player_transform->pos.x = max_x;

    spawn_dash_dust(dx);

    if (player_anim) {
      if (dx < 0.0f) {
        player_anim->set_state("left");
      } else if (dx > 0.0f) {
        player_anim->set_state("right");
      }
    }
  }

  void spawn_dash_dust(float dx) {
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    dust_timer -= dt;
    if (dust_timer > 0.0f) return;
    dust_timer = dust_period;

    if (!player_transform) return;
    const float direction = dx < 0.0f ? -1.0f : 1.0f;
    const glm::vec2 foot{
        player_transform->pos.x + player_size.x * (direction < 0.0f ? 0.25f : 0.75f),
        player_transform->pos.y + player_size.y * 0.92f,
    };
    vfx::SporeConfig config{};
    config.color = glm::vec4{0.35f, 0.2f, 0.45f, 0.4f};
    config.lifetime = 0.22f;
    config.start_radius = 2.5f;
    config.end_radius = 6.0f;
    config.velocity = glm::vec2{direction * 18.0f, 20.0f};
    config.layer = 2;
    vfx::spawn_spore(foot, config);
  }

  float step_px = 0.0f;
  float dust_timer = 0.0f;
  float dust_period = 0.08f;
  bool deploy_pressed_last = false;
  bool fire_pressed_last = false;
};

inline collision::TriggerObject* make_player_trigger() {
  return arena::create<collision::TriggerObject>(
      "mushroom_catch_handler",
      [](ecs::Entity*, collision::ColliderObject* collider) {
        if (!collider) return;
        auto* entity = collider->get_entity();
        if (!entity || entity->is_pending_deletion()) return;
        if (vfx::is_catch_animating(entity)) return;
        auto* sprite = entity->get<render_system::SpriteRenderable>();
        const std::string type = sprite ? engine::resources::texture_name(sprite->texture_id) : "";
        float catch_x = std::numeric_limits<float>::quiet_NaN();
        if (player_transform) {
          catch_x = player_transform->pos.x + player_size.x * 0.5f;
        }
        levels::on_mushroom_caught(type, entity, catch_x, false);
      });
}

inline void reset_for_new_level() {
  if (!player_transform) return;
  glm::vec2 center = shrooms::screen::norm_to_pixels(glm::vec2{0.0f, -0.6f});
  player_transform->pos = shrooms::screen::center_to_top_left(center, player_size);
  reset_familiars();
}

inline void init() {
  player_entity = arena::create<ecs::Entity>();
  player_size = shrooms::texture_sizing::from_reference_width("witch_right_1", 34.0f);

  player_transform = arena::create<transform::NoRotationTransform>();
  glm::vec2 center = shrooms::screen::norm_to_pixels(glm::vec2{0.0f, -0.6f});
  player_transform->pos = shrooms::screen::center_to_top_left(center, player_size);
  player_entity->add(player_transform);

  player_entity->add(arena::create<geometry::Quad>(
      "player_collider",
      std::vector<glm::vec2>{
          glm::vec2{0.0f, 0.0f},
          glm::vec2{player_size.x, 0.0f},
          glm::vec2{0.0f, player_size.y},
          glm::vec2{player_size.x, player_size.y},
      }));

  player_entity->add(make_player_trigger());

  const engine::TextureId tex_id =
      engine::resources::register_texture("witch_right_1");
  player_entity->add(arena::create<render_system::SpriteRenderable>(tex_id, player_size));

  std::map<std::string, std::vector<animation::SpriteFrame>> clips{};
  clips["left"] = {animation::SpriteFrame{engine::resources::register_texture("witch_left_1"),
                                          0.15f},
                   animation::SpriteFrame{engine::resources::register_texture("witch_left_2"),
                                          0.15f}};
  clips["right"] = {animation::SpriteFrame{engine::resources::register_texture("witch_right_1"),
                                           0.15f},
                    animation::SpriteFrame{engine::resources::register_texture("witch_right_2"),
                                           0.15f}};

  player_anim = arena::create<animation::SpriteAnimation>(std::move(clips), "right");
  player_entity->add(player_anim);

  const float step_px = shrooms::screen::scale_to_pixels(glm::vec2{0.02f, 0.0f}).x * 0.5f;
  player_controller = arena::create<PlayerController>(step_px);
  player_entity->add(player_controller);
  player_vibe = arena::create<PlayerVibe>();
  player_vibe->bob_amp_px = shrooms::screen::scale_to_pixels(glm::vec2{0.0f, 0.01f}).y;
  player_vibe->sway_amp_px = shrooms::screen::scale_to_pixels(glm::vec2{0.01f, 0.0f}).x;
  player_vibe->phase = 1.3f;
  player_entity->add(player_vibe);
  camera_shake::attach(player_entity, camera_shake::AxisMode::XOnly, 1.8f);
  player_entity->add(arena::create<scene::SceneObject>("main"));

  init_familiars();
}

}  // namespace player
