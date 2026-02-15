#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/defer/deferred_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "engine/resource_ids.h"

#include "shrooms_screen.hpp"

namespace countdown {

struct Config {
  float scale = 0.15f;
  int layer = 120;
} config;

inline ecs::Entity* digit_sprite = nullptr;
inline bool active = false;
inline uint64_t serial = 0;
inline std::function<void()> on_done = nullptr;
inline std::string target_scene = "main";

inline void clear_digit() {
  if (!digit_sprite) return;
  digit_sprite->mark_deleted();
  digit_sprite = nullptr;
}

inline void finish(uint64_t token) {
  if (token != serial) return;
  active = false;
  clear_digit();
  if (on_done) {
    auto done = std::move(on_done);
    on_done = nullptr;
    done();
  }
}

inline void show_digit(int digit, uint64_t token) {
  if (token != serial) return;
  clear_digit();
  if (digit <= 0) {
    finish(token);
    return;
  }

  digit_sprite = arena::create<ecs::Entity>();
  const glm::vec2 size =
      shrooms::screen::scale_to_pixels(glm::vec2{config.scale, config.scale});
  const glm::vec2 center = shrooms::screen::norm_to_pixels(glm::vec2{0.0f, 0.0f});
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = shrooms::screen::center_to_top_left(center, size);
  digit_sprite->add(transform);
  digit_sprite->add(arena::create<layers::ConstLayer>(config.layer));
  const std::string tex_name = std::string("digits_") + std::to_string(digit);
  const engine::TextureId tex_id = engine::resources::register_texture(tex_name);
  digit_sprite->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  digit_sprite->add(arena::create<scene::SceneObject>(target_scene));

  deferred::fire_deferred([digit, token]() { show_digit(digit - 1, token); }, 1000);
}

inline void start(const std::string& scene_name, int seconds,
                  std::function<void()> done = nullptr) {
  serial++;
  target_scene = scene_name;
  on_done = std::move(done);
  active = true;
  show_digit(seconds, serial);
}

inline bool is_active() { return active; }

inline void cancel() {
  serial++;
  active = false;
  clear_digit();
  on_done = nullptr;
}

}  // namespace countdown
