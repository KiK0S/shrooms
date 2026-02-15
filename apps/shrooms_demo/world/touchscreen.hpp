#pragma once

#include <cmath>
#include <cstdio>
#include <optional>
#include <utility>
#include <vector>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/input/input_system.hpp"
#include "systems/render/render_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "systems/layer/layered_object.hpp"

#include "shrooms_screen.hpp"
#include "shrooms_scenes.hpp"

namespace touchscreen {

struct JoystickConfig {
  glm::vec2 center_norm = glm::vec2(-0.6f, -0.6f);
  float outer_radius_norm = 0.25f;
  float inner_radius_norm = 0.12f;
  float activation_margin = 1.1f;
  int layer = 8;
} config;

struct FireButtonConfig {
  glm::vec2 center_norm = glm::vec2(0.7f, -0.6f);
  float radius_norm = 0.12f;
  int layer = 8;
} fire_config;

struct DeployButtonConfig {
  glm::vec2 center_norm = glm::vec2(0.7f, -0.2f);
  float radius_norm = 0.1f;
  int layer = 8;
} deploy_config;

inline ecs::Entity* joystick_outer = nullptr;
inline ecs::Entity* joystick_inner = nullptr;
inline ecs::Entity* fire_button = nullptr;
inline ecs::Entity* deploy_button = nullptr;
inline glm::vec2 joystick_value{0.0f, 0.0f};
inline std::optional<int> active_pointer{};
inline bool initialized = false;
inline bool activated = false;
inline bool fire_pressed = false;
inline bool deploy_pressed = false;
inline bool logged_activation = false;

void init();

inline glm::vec2 center_px() {
  return shrooms::screen::norm_to_pixels(config.center_norm);
}

inline float outer_radius_px() {
  return config.outer_radius_norm * 0.5f * static_cast<float>(shrooms::screen::view_width);
}

inline float inner_radius_px() {
  return config.inner_radius_norm * 0.5f * static_cast<float>(shrooms::screen::view_width);
}

inline glm::vec2 fire_center_px() {
  return shrooms::screen::norm_to_pixels(fire_config.center_norm);
}

inline float fire_radius_px() {
  return fire_config.radius_norm * 0.5f * static_cast<float>(shrooms::screen::view_width);
}

inline glm::vec2 deploy_center_px() {
  return shrooms::screen::norm_to_pixels(deploy_config.center_norm);
}

inline float deploy_radius_px() {
  return deploy_config.radius_norm * 0.5f * static_cast<float>(shrooms::screen::view_width);
}

inline bool is_fire_pressed(
    const std::vector<std::pair<int, glm::vec2>>& points) {
  const glm::vec2 fire_center = fire_center_px();
  const float fire_radius = fire_radius_px();
  const float fire_radius_sq = fire_radius * fire_radius;
  for (const auto& [id, pos] : points) {
    const glm::vec2 to_fire = pos - fire_center;
    if (to_fire.x * to_fire.x + to_fire.y * to_fire.y <= fire_radius_sq) {
      return true;
    }
  }
  return false;
}

inline bool is_deploy_pressed(
    const std::vector<std::pair<int, glm::vec2>>& points) {
  const glm::vec2 deploy_center = deploy_center_px();
  const float deploy_radius = deploy_radius_px();
  const float deploy_radius_sq = deploy_radius * deploy_radius;
  for (const auto& [id, pos] : points) {
    const glm::vec2 to_deploy = pos - deploy_center;
    if (to_deploy.x * to_deploy.x + to_deploy.y * to_deploy.y <= deploy_radius_sq) {
      return true;
    }
  }
  return false;
}

inline void update_inner_sprite(const glm::vec2& offset) {
  if (!joystick_inner) return;
  auto* transform = joystick_inner->get<transform::NoRotationTransform>();
  if (!transform) return;
  const glm::vec2 size{inner_radius_px() * 2.0f, inner_radius_px() * 2.0f};
  transform->pos = shrooms::screen::center_to_top_left(center_px() + offset, size);
}

inline void reset() {
  joystick_value = glm::vec2{0.0f, 0.0f};
  active_pointer.reset();
  fire_pressed = false;
  deploy_pressed = false;
  update_inner_sprite(glm::vec2{0.0f, 0.0f});
}

inline bool should_activate() {
  if (activated) return true;
  for (const auto& evt : input::events()) {
    if (evt.kind != engine::InputKind::PointerDown) continue;
    if (evt.pointer_id == 0) continue;
    activated = true;
    if (!logged_activation) {
      logged_activation = true;
      std::fprintf(stderr, "touchscreen: activated by pointer %d at %.1f, %.1f\n", evt.pointer_id,
                   evt.x, evt.y);
    }
    return true;
  }
  return false;
}

struct JoystickSystem : public dynamic::DynamicObject {
  JoystickSystem() : dynamic::DynamicObject() {}
  ~JoystickSystem() override { Component::component_count--; }

  void update() override {
    if (shrooms::scenes::menu && shrooms::scenes::menu->is_active) {
      reset();
      return;
    }

    if (!should_activate()) {
      reset();
      return;
    }

    const auto points = input::active_pointers_with_ids();
    if (points.empty()) {
      reset();
      return;
    }

    if (!initialized) {
      init();
      if (!initialized) return;
    }

    fire_pressed = is_fire_pressed(points);
    deploy_pressed = is_deploy_pressed(points);

    std::optional<glm::vec2> active_pos{};
    if (active_pointer) {
      for (const auto& [id, pos] : points) {
        if (id == *active_pointer) {
          active_pos = pos;
          break;
        }
      }
    }

    if (!active_pos) {
      const float radius = outer_radius_px() * config.activation_margin;
      const float radius_sq = radius * radius;
      for (const auto& [id, pos] : points) {
        const glm::vec2 delta = pos - center_px();
        if (delta.x * delta.x + delta.y * delta.y <= radius_sq) {
          active_pointer = id;
          active_pos = pos;
          break;
        }
      }
    }

    if (!active_pos) {
      joystick_value = glm::vec2{0.0f, 0.0f};
      active_pointer.reset();
      update_inner_sprite(glm::vec2{0.0f, 0.0f});
      return;
    }

    glm::vec2 delta = *active_pos - center_px();
    const float max_radius = outer_radius_px();
    const float len = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (len > max_radius && len > 0.0001f) {
      delta = delta / len * max_radius;
    }

    joystick_value = (max_radius > 0.0f) ? (delta / max_radius) : glm::vec2{0.0f, 0.0f};
    update_inner_sprite(delta);
  }
};

inline JoystickSystem joystick_system{};

inline void init() {
  if (initialized) return;
  const glm::vec2 outer_size{outer_radius_px() * 2.0f, outer_radius_px() * 2.0f};
  const glm::vec2 inner_size{inner_radius_px() * 2.0f, inner_radius_px() * 2.0f};
  const glm::vec2 center = center_px();

  joystick_outer = arena::create<ecs::Entity>();
  auto* outer_transform = arena::create<transform::NoRotationTransform>();
  outer_transform->pos = shrooms::screen::center_to_top_left(center, outer_size);
  joystick_outer->add(outer_transform);
  joystick_outer->add(arena::create<layers::ConstLayer>(config.layer));
  joystick_outer->add(arena::create<render_system::CircleRenderable>(
      outer_radius_px(), engine::UIColor{0.2f, 0.2f, 0.2f, 0.6f}));
  joystick_outer->add(arena::create<scene::SceneObject>("main"));

  joystick_inner = arena::create<ecs::Entity>();
  auto* inner_transform = arena::create<transform::NoRotationTransform>();
  inner_transform->pos = shrooms::screen::center_to_top_left(center, inner_size);
  joystick_inner->add(inner_transform);
  joystick_inner->add(arena::create<layers::ConstLayer>(config.layer + 1));
  joystick_inner->add(arena::create<render_system::CircleRenderable>(
      inner_radius_px(), engine::UIColor{0.6f, 0.6f, 0.6f, 0.8f}));
  joystick_inner->add(arena::create<scene::SceneObject>("main"));

  fire_button = arena::create<ecs::Entity>();
  const glm::vec2 fire_size{fire_radius_px() * 2.0f, fire_radius_px() * 2.0f};
  auto* fire_transform = arena::create<transform::NoRotationTransform>();
  fire_transform->pos = shrooms::screen::center_to_top_left(fire_center_px(), fire_size);
  fire_button->add(fire_transform);
  fire_button->add(arena::create<layers::ConstLayer>(fire_config.layer));
  fire_button->add(arena::create<render_system::CircleRenderable>(
      fire_radius_px(), engine::UIColor{0.9f, 0.4f, 0.4f, 0.7f}));
  fire_button->add(arena::create<scene::SceneObject>("main"));

  deploy_button = arena::create<ecs::Entity>();
  const glm::vec2 deploy_size{deploy_radius_px() * 2.0f, deploy_radius_px() * 2.0f};
  auto* deploy_transform = arena::create<transform::NoRotationTransform>();
  deploy_transform->pos = shrooms::screen::center_to_top_left(deploy_center_px(), deploy_size);
  deploy_button->add(deploy_transform);
  deploy_button->add(arena::create<layers::ConstLayer>(deploy_config.layer));
  deploy_button->add(arena::create<render_system::CircleRenderable>(
      deploy_radius_px(), engine::UIColor{0.5f, 0.5f, 0.9f, 0.7f}));
  deploy_button->add(arena::create<scene::SceneObject>("main"));
  initialized = true;
}

}  // namespace touchscreen
