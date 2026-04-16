#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/animation/sprite_animation.hpp"
#include "systems/color/color_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"

namespace score_hud {

struct Config {
  glm::vec2 panel_reference_size = glm::vec2(0.0f, 0.0f);
  glm::vec2 face_reference_size = glm::vec2(0.0f, 0.0f);
  glm::vec2 face_offset = glm::vec2(-0.05f, -0.11f);
  glm::vec2 score_offset = glm::vec2(-0.005f, -0.04f);
  float score_font_px = 20.0f;
  glm::vec4 score_color = glm::vec4(1.0f);
  int layer = 1;
} config;

inline ecs::Entity* face_icon = nullptr;
inline ecs::Entity* panel = nullptr;
inline ecs::Entity* score_text_entity = nullptr;
inline transform::NoRotationTransform* score_text_transform = nullptr;
inline text::TextObject* score_text = nullptr;
inline color::OneColor* score_text_color = nullptr;
inline int current_score = 0;

inline glm::vec2 resolve_reference_size(const glm::vec2& configured,
                                        std::string_view texture_name,
                                        float fallback_width) {
  if (configured.x > 0.0f && configured.y > 0.0f) return configured;
  const glm::vec2 measured = shrooms::texture_sizing::reference_size(texture_name);
  if (measured.x > 0.0f && measured.y > 0.0f) return measured;
  return shrooms::texture_sizing::reference_size_from_width(texture_name, fallback_width);
}

inline glm::vec2 panel_top_left_norm() { return glm::vec2{-1.0f, 1.0f}; }

inline glm::vec2 panel_size_px() {
  const glm::vec2 ref_size = resolve_reference_size(config.panel_reference_size, "menu_face", 107.0f);
  return shrooms::texture_sizing::from_reference_size(ref_size);
}

inline glm::vec2 panel_center_px() {
  const glm::vec2 top_left = shrooms::screen::norm_to_pixels(panel_top_left_norm());
  return top_left + panel_size_px() * 0.5f;
}

inline glm::vec2 panel_center_norm() {
  const glm::vec2 size = panel_size_px();
  const glm::vec2 top_left_norm = panel_top_left_norm();
  return glm::vec2{
      top_left_norm.x + (size.x / static_cast<float>(shrooms::screen::view_width)),
      top_left_norm.y - (size.y / static_cast<float>(shrooms::screen::view_height)),
  };
}

inline glm::vec2 score_anchor_px() {
  return panel_center_px() + shrooms::screen::scale_to_pixels(config.score_offset);
}

inline void update_score_layout() {
  if (!score_text || !score_text_transform) return;
  const std::string value = std::to_string(current_score);
  score_text->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, config.score_font_px);
  const glm::vec2 size{layout.width, layout.height};
  score_text_transform->pos = score_anchor_px() - size * 0.5f;
}

inline void set_score(int score_value) {
  current_score = score_value;
  update_score_layout();
}

inline int score() { return current_score; }

inline void reset_hud() {
  const glm::vec2 panel_top_left_px = shrooms::screen::norm_to_pixels(panel_top_left_norm());
  const glm::vec2 panel_size = panel_size_px();
  const glm::vec2 panel_center = panel_top_left_px + panel_size * 0.5f;
  const glm::vec2 center_norm = panel_center_norm();

  if (panel) {
    panel->mark_deleted();
  }
  panel = arena::create<ecs::Entity>();
  {
    auto* transform = arena::create<transform::NoRotationTransform>();
    transform->pos = shrooms::screen::center_to_top_left(panel_center, panel_size);
    panel->add(transform);
    panel->add(arena::create<layers::ConstLayer>(config.layer));
    const engine::TextureId tex_id = engine::resources::register_texture("menu_face");
    panel->add(arena::create<render_system::SpriteRenderable>(tex_id, panel_size));
    panel->add(arena::create<scene::SceneObject>("main"));
  }

  if (face_icon) {
    face_icon->mark_deleted();
  }
  face_icon = arena::create<ecs::Entity>();
  {
    const glm::vec2 ref_size =
        resolve_reference_size(config.face_reference_size, "face_mini_1", 24.0f);
    const glm::vec2 size = shrooms::texture_sizing::from_reference_size(ref_size);
    const glm::vec2 center = shrooms::screen::norm_to_pixels(center_norm + config.face_offset);
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

  if (score_text_entity) {
    score_text_entity->mark_deleted();
  }
  score_text_entity = arena::create<ecs::Entity>();
  score_text_transform = arena::create<transform::NoRotationTransform>();
  score_text_entity->add(score_text_transform);
  score_text_entity->add(arena::create<layers::ConstLayer>(config.layer + 1));
  score_text = arena::create<text::TextObject>("0", config.score_font_px);
  score_text_entity->add(score_text);
  score_text_color = arena::create<color::OneColor>(config.score_color);
  score_text_entity->add(score_text_color);
  score_text_entity->add(arena::create<scene::SceneObject>("main"));
  update_score_layout();
}

inline void init() {
  current_score = 0;
  reset_hud();
}

inline void reset_for_run() { set_score(0); }

}  // namespace score_hud
