#pragma once

#include <algorithm>
#include <cmath>

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/transformation/transform_object.hpp"

#include "shrooms_screen.hpp"

namespace camera_shake {

struct Config {
  float max_offset_px = 6.0f;
  float decay = 3.0f;
  float frequency = 24.0f;
} config;

enum class AxisMode {
  Full,
  XOnly,
};

struct ShakeTarget;
COMPONENT_VECTOR(ShakeTarget, shake_targets);

struct ShakeTarget : public ecs::Component {
  explicit ShakeTarget(AxisMode axis_mode, float strength)
      : ecs::Component(), axis(axis_mode), strength(strength) {
    shake_targets.push_back(this);
  }
  ~ShakeTarget() override { Component::component_count--; }

  AxisMode axis = AxisMode::Full;
  float strength = 1.0f;
  glm::vec2 last_offset{0.0f, 0.0f};
  DETACH_VECTOR(ShakeTarget, shake_targets)
};

inline ShakeTarget* attach(ecs::Entity* entity, AxisMode axis = AxisMode::Full,
                           float strength = 1.0f) {
  if (!entity) return nullptr;
  auto* target = arena::create<ShakeTarget>(axis, strength);
  entity->add(target);
  return target;
}

inline void clear_offsets() {
  for (auto* target : shake_targets) {
    if (!target) continue;
    auto* entity = target->get_entity();
    if (!entity || entity->is_pending_deletion()) continue;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) continue;
    if (target->last_offset.x != 0.0f || target->last_offset.y != 0.0f) {
      transform->translate(
          glm::vec2{-target->last_offset.x, -target->last_offset.y});
      target->last_offset = glm::vec2{0.0f, 0.0f};
    }
  }
}

inline void apply_offsets(const glm::vec2& offset) {
  for (auto* target : shake_targets) {
    if (!target) continue;
    auto* entity = target->get_entity();
    if (!entity || entity->is_pending_deletion()) continue;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) continue;
    glm::vec2 applied = offset * target->strength;
    if (target->axis == AxisMode::XOnly) {
      applied.y = 0.0f;
    }
    transform->translate(applied - target->last_offset);
    target->last_offset = applied;
  }
}

struct CameraShake : public dynamic::DynamicObject {
  CameraShake() : dynamic::DynamicObject() {}
  ~CameraShake() override { Component::component_count--; }

  void add_trauma(float amount) {
    trauma = std::min(1.0f, trauma + amount);
  }

  void update() override {
    auto* active = scene::get_active_scene();
    if (!active || active->get_name() != "main") {
      trauma = 0.0f;
      clear_offsets();
      return;
    }
    if (active->is_paused_state()) {
      trauma = 0.0f;
      clear_offsets();
      return;
    }

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    trauma = std::max(0.0f, trauma - config.decay * dt);
    if (trauma <= 0.001f) {
      clear_offsets();
      return;
    }

    const float t = static_cast<float>(ecs::context().time_seconds);
    const float shake = trauma * trauma;
    const float nx = std::sin(t * config.frequency + phase_x);
    const float ny = std::sin(t * config.frequency * 0.93f + phase_y);
    const float offset = config.max_offset_px * shake;
    apply_offsets(glm::vec2{nx * offset, ny * offset});
  }

  float trauma = 0.0f;
  float phase_x = 0.0f;
  float phase_y = 0.0f;
};

inline CameraShake* controller = nullptr;

inline void init() {
  const glm::vec2 shake_px = shrooms::screen::scale_to_pixels(glm::vec2{0.012f, 0.0f});
  config.max_offset_px = std::max(6.0f, shake_px.x);

  auto* entity = arena::create<ecs::Entity>();
  controller = arena::create<CameraShake>();
  controller->phase_x = static_cast<float>(rnd::get_double(0.0, 6.28318));
  controller->phase_y = static_cast<float>(rnd::get_double(0.0, 6.28318));
  entity->add(controller);
}

inline void add_trauma(float amount) {
  if (!controller) return;
  controller->add_trauma(amount);
}

inline void reset() {
  if (controller) {
    controller->trauma = 0.0f;
  }
  clear_offsets();
}

}  // namespace camera_shake
