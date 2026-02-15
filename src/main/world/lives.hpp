#pragma once

#include <array>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/animation/sprite_animation.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"

namespace lives {

struct Config {
  glm::vec2 panel_reference_size = glm::vec2(0.0f, 0.0f);
  glm::vec2 face_reference_size = glm::vec2(0.0f, 0.0f);
  glm::vec2 heart_reference_size = glm::vec2(0.0f, 0.0f);
  float horizontal_spacing = 0.05f;
  glm::vec2 hearts_center_offset = glm::vec2(-0.005f, -0.04f);
  glm::vec2 face_offset = glm::vec2(-0.05f, -0.11f);
  int layer = 1;
} config;

constexpr int kMaxLives = 3;
inline int current_lives = kMaxLives;
inline std::array<ecs::Entity*, kMaxLives> heart_icons{};
inline ecs::Entity* face_icon = nullptr;
inline ecs::Entity* panel = nullptr;

inline glm::vec2 resolve_reference_size(const glm::vec2& configured,
                                        std::string_view texture_name,
                                        float fallback_width) {
  if (configured.x > 0.0f && configured.y > 0.0f) return configured;
  const glm::vec2 measured = shrooms::texture_sizing::reference_size(texture_name);
  if (measured.x > 0.0f && measured.y > 0.0f) return measured;
  return shrooms::texture_sizing::reference_size_from_width(texture_name, fallback_width);
}

inline void remove_life() {
  if (current_lives <= 0) return;
  if (heart_icons[current_lives - 1]) {
    heart_icons[current_lives - 1]->mark_deleted();
  }
  current_lives--;
}

inline void reset_lives() {
  current_lives = kMaxLives;
  const glm::vec2 panel_top_left_norm = glm::vec2{-1.0f, 1.0f};
  const glm::vec2 panel_top_left_px = shrooms::screen::norm_to_pixels(panel_top_left_norm);
  glm::vec2 panel_center_px = panel_top_left_px;
  glm::vec2 panel_center_norm = panel_top_left_norm;

  if (panel) {
    panel->mark_deleted();
  }
  panel = arena::create<ecs::Entity>();
  {
    const glm::vec2 ref_size =
        resolve_reference_size(config.panel_reference_size, "menu_face", 107.0f);
    const glm::vec2 size = shrooms::texture_sizing::from_reference_size(ref_size);
    panel_center_px = panel_top_left_px + size * 0.5f;
    panel_center_norm = glm::vec2{
        panel_top_left_norm.x + (size.x / static_cast<float>(shrooms::screen::view_width)),
        panel_top_left_norm.y - (size.y / static_cast<float>(shrooms::screen::view_height)),
    };
    auto* transform = arena::create<transform::NoRotationTransform>();
    transform->pos = shrooms::screen::center_to_top_left(panel_center_px, size);
    panel->add(transform);
    panel->add(arena::create<layers::ConstLayer>(config.layer));
    const engine::TextureId tex_id = engine::resources::register_texture("menu_face");
    panel->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
    panel->add(arena::create<scene::SceneObject>("main"));
  }

  const glm::vec2 hearts_center_offset_px =
      shrooms::screen::scale_to_pixels(config.hearts_center_offset);
  const glm::vec2 hearts_row_center_px = panel_center_px + hearts_center_offset_px;

  if (face_icon) {
    face_icon->mark_deleted();
  }
  face_icon = arena::create<ecs::Entity>();
  {
    const glm::vec2 ref_size =
        resolve_reference_size(config.face_reference_size, "face_mini_1", 24.0f);
    const glm::vec2 size = shrooms::texture_sizing::from_reference_size(ref_size);
    const glm::vec2 center = shrooms::screen::norm_to_pixels(
        panel_center_norm + config.face_offset);
    auto* transform = arena::create<transform::NoRotationTransform>();
    transform->pos = shrooms::screen::center_to_top_left(center, size);
    face_icon->add(transform);
    face_icon->add(arena::create<layers::ConstLayer>(config.layer));
    const engine::TextureId frame_1 = engine::resources::register_texture("face_mini_1");
    const engine::TextureId frame_2 = engine::resources::register_texture("face_mini_2");
    face_icon->add(arena::create<render_system::SpriteRenderable>(frame_1, size));
    std::map<std::string, std::vector<animation::SpriteFrame>> clips{};
    clips["idle"] = {animation::SpriteFrame{frame_1, 0.25f},
                     animation::SpriteFrame{frame_2, 0.25f}};
    face_icon->add(arena::create<animation::SpriteAnimation>(std::move(clips), "idle"));
    face_icon->add(arena::create<scene::SceneObject>("main"));
  }

  for (int i = 0; i < kMaxLives; ++i) {
    if (heart_icons[i]) {
      heart_icons[i]->mark_deleted();
    }
    heart_icons[i] = arena::create<ecs::Entity>();
    const glm::vec2 ref_size =
        resolve_reference_size(config.heart_reference_size, "heart", 11.0f);
    const glm::vec2 size = shrooms::texture_sizing::from_reference_size(ref_size);
    const float centered_index =
        static_cast<float>(i) - static_cast<float>(kMaxLives - 1) * 0.5f;
    const glm::vec2 center =
        hearts_row_center_px +
        shrooms::screen::scale_to_pixels(glm::vec2{config.horizontal_spacing * centered_index,
                                                    0.0f});
    auto* transform = arena::create<transform::NoRotationTransform>();
    transform->pos = shrooms::screen::center_to_top_left(center, size);
    heart_icons[i]->add(transform);
    heart_icons[i]->add(arena::create<layers::ConstLayer>(config.layer));
    const engine::TextureId tex_id = engine::resources::register_texture("heart");
    heart_icons[i]->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
    heart_icons[i]->add(arena::create<scene::SceneObject>("main"));
  }
}

inline void init() { reset_lives(); }

}  // namespace lives
