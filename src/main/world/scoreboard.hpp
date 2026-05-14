#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"

namespace scoreboard {

struct Config {
  glm::vec2 icon_scale = glm::vec2(0.05f, 0.05f);
  float icon_x = 0.66f;
  float score_x = 0.82f;
  float row_spacing = 0.115f;
  glm::vec2 center_intro_norm = glm::vec2{0.0f, -0.18f};
  float default_move_duration = 0.65f;
  float shake_duration = 0.45f;
  float shake_amplitude_px = 10.0f;
  float shake_frequency = 38.0f;
  float text_font_px = 20.0f;
  glm::vec4 text_color = glm::vec4(1.0f);
  int layer = 1;
} config;

enum class LayoutState {
  Hidden,
  CenterIntro,
  Corner,
};

struct Entry {
  std::string name;
  int current = 0;
  int target = 0;
  ecs::Entity* icon = nullptr;
  transform::NoRotationTransform* icon_transform = nullptr;
  ecs::Entity* score_text_entity = nullptr;
  transform::NoRotationTransform* score_text_transform = nullptr;
  text::TextObject* score_text = nullptr;
  size_t row_index = 0;
};

inline std::vector<Entry> entries{};
inline std::unordered_map<std::string, size_t> entry_index{};
inline std::vector<std::pair<std::string, int>> current_recipe{};
inline std::string objective_word{};
inline ecs::Entity* panel = nullptr;
inline transform::NoRotationTransform* panel_transform = nullptr;
inline size_t active_rows = 0;
inline LayoutState layout_state = LayoutState::Corner;
inline glm::vec2 current_panel_center_norm{0.0f, 0.0f};
inline glm::vec2 animation_start_norm{0.0f, 0.0f};
inline glm::vec2 animation_target_norm{0.0f, 0.0f};
inline float animation_elapsed = 0.0f;
inline float animation_duration = 0.0f;
inline bool animation_active = false;
inline float shake_elapsed = 0.0f;
inline float shake_duration = 0.0f;
inline float active_shake_amplitude_px = 0.0f;
inline glm::vec2 shake_offset_px{0.0f, 0.0f};

inline float clamp01(float value) {
  return std::min(1.0f, std::max(0.0f, value));
}

inline float ease_in_out(float value) {
  const float t = clamp01(value);
  return t * t * (3.0f - 2.0f * t);
}

inline glm::vec2 panel_reference_size() {
  const glm::vec2 measured = shrooms::texture_sizing::reference_size("menu_scoreboard");
  if (measured.x > 0.0f && measured.y > 0.0f) return measured;
  return shrooms::texture_sizing::reference_size_from_width("menu_scoreboard", 64.0f);
}

inline glm::vec2 panel_size_px() {
  return shrooms::texture_sizing::from_reference_size(panel_reference_size());
}

inline glm::vec2 corner_panel_center_norm() {
  const glm::vec2 size = panel_size_px();
  return glm::vec2{
      1.0f - (size.x / static_cast<float>(shrooms::screen::view_width)),
      1.0f - (size.y / static_cast<float>(shrooms::screen::view_height)),
  };
}

inline glm::vec2 target_center_norm(LayoutState state) {
  switch (state) {
    case LayoutState::Hidden:
    case LayoutState::Corner:
      return corner_panel_center_norm();
    case LayoutState::CenterIntro:
      return config.center_intro_norm;
  }
  return corner_panel_center_norm();
}

inline glm::vec2 panel_center_norm() {
  if (current_panel_center_norm.x == 0.0f && current_panel_center_norm.y == 0.0f &&
      layout_state == LayoutState::Corner) {
    current_panel_center_norm = corner_panel_center_norm();
  }
  return current_panel_center_norm;
}

inline glm::vec2 current_panel_center_px() {
  return shrooms::screen::norm_to_pixels(panel_center_norm()) + shake_offset_px;
}

inline glm::vec2 corner_relative_norm(float x_norm, float y_norm) {
  const glm::vec2 corner = corner_panel_center_norm();
  return panel_center_norm() + glm::vec2{x_norm - corner.x, y_norm - corner.y};
}

inline float row_center_y_norm(size_t row_index) {
  const float panel_center_y = panel_center_norm().y;
  if (active_rows == 0) return panel_center_y;
  const float middle = static_cast<float>(active_rows - 1) * 0.5f;
  return panel_center_y + (static_cast<float>(row_index) - middle) * config.row_spacing;
}

inline glm::vec2 row_icon_position(size_t row_index) {
  const glm::vec2 norm = corner_relative_norm(config.icon_x, row_center_y_norm(row_index));
  return shrooms::screen::norm_to_pixels(norm) + shake_offset_px;
}

inline glm::vec2 row_score_center(size_t row_index) {
  const glm::vec2 norm = corner_relative_norm(config.score_x, row_center_y_norm(row_index));
  return shrooms::screen::norm_to_pixels(norm) + shake_offset_px;
}

inline std::string icon_texture_name(const std::string& name) {
  if (name == "mukhomor" || name == "lisi4ka" || name == "borovik") {
    return name + "_small";
  }
  return name;
}

inline std::string score_text_value(const Entry& entry) {
  const std::string counter = std::to_string(entry.current) + "/" + std::to_string(entry.target);
  if (objective_word.empty()) return counter;
  return objective_word + "\n" + counter;
}

inline void destroy_entry(Entry& entry) {
  if (entry.icon) {
    entry.icon->mark_deleted();
    entry.icon = nullptr;
  }
  if (entry.score_text_entity) {
    entry.score_text_entity->mark_deleted();
    entry.score_text_entity = nullptr;
  }
  entry.score_text_transform = nullptr;
  entry.score_text = nullptr;
}

inline void clear_entries() {
  for (auto& entry : entries) {
    destroy_entry(entry);
  }
  entries.clear();
  entry_index.clear();
  active_rows = 0;
}

inline void reset_panel() {
  if (panel) {
    panel->mark_deleted();
  }
  panel = arena::create<ecs::Entity>();
  panel_transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 size = panel_size_px();
  panel_transform->pos = shrooms::screen::center_to_top_left(current_panel_center_px(), size);
  panel->add(panel_transform);
  panel->add(arena::create<layers::ConstLayer>(config.layer));
  const engine::TextureId tex_id = engine::resources::register_texture("menu_scoreboard");
  panel->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  panel->add(arena::create<scene::SceneObject>("main"));
}

inline void hide_panel() {
  if (!panel) return;
  panel->mark_deleted();
  panel = nullptr;
  panel_transform = nullptr;
}

inline void ensure_panel() {
  if (!panel) {
    reset_panel();
  }
}

inline ecs::Entity* create_sprite(const std::string& texture_name, const glm::vec2& center,
                                  const glm::vec2& size, int layer) {
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = shrooms::screen::center_to_top_left(center, size);
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(layer));
  const engine::TextureId tex_id = engine::resources::register_texture(texture_name);
  entity->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  entity->add(arena::create<scene::SceneObject>("main"));
  return entity;
}

inline void update_entry_text(Entry& entry);

inline void apply_layout() {
  if (current_recipe.empty()) return;

  if (panel_transform) {
    const glm::vec2 size = panel_size_px();
    panel_transform->pos = shrooms::screen::center_to_top_left(current_panel_center_px(), size);
  }

  for (auto& entry : entries) {
    if (entry.icon_transform && entry.icon) {
      const auto* sprite = entry.icon->get<render_system::SpriteRenderable>();
      const glm::vec2 size = sprite ? sprite->size : glm::vec2{0.0f, 0.0f};
      entry.icon_transform->pos =
          shrooms::screen::center_to_top_left(row_icon_position(entry.row_index), size);
    }
    update_entry_text(entry);
  }
}

inline void update_entry_text(Entry& entry) {
  if (!entry.score_text || !entry.score_text_transform) return;
  const std::string value = score_text_value(entry);
  entry.score_text->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, config.text_font_px);
  const glm::vec2 size{layout.width, layout.height};
  const glm::vec2 center = row_score_center(entry.row_index);
  entry.score_text_transform->pos = center - size * 0.5f;
}

inline void create_entry_visuals(Entry& entry, size_t index) {
  const std::string icon_name = icon_texture_name(entry.name);
  const float icon_width =
      shrooms::screen::scale_to_pixels(glm::vec2{config.icon_scale.x, 0.0f}).x;
  const glm::vec2 icon_size =
      shrooms::texture_sizing::from_width_px(icon_name, icon_width);
  const glm::vec2 icon_center = row_icon_position(index);
  entry.icon = create_sprite(icon_name, icon_center, icon_size, config.layer + 1);
  entry.icon_transform = entry.icon ? entry.icon->get<transform::NoRotationTransform>() : nullptr;
  entry.icon->bind();

  entry.row_index = index;
  entry.score_text_entity = arena::create<ecs::Entity>();
  entry.score_text_transform = arena::create<transform::NoRotationTransform>();
  entry.score_text_entity->add(entry.score_text_transform);
  entry.score_text_entity->add(arena::create<layers::ConstLayer>(config.layer + 1));
  entry.score_text = arena::create<text::TextObject>("", config.text_font_px);
  entry.score_text_entity->add(entry.score_text);
  entry.score_text_entity->add(arena::create<color::OneColor>(config.text_color));
  entry.score_text_entity->add(arena::create<scene::SceneObject>("main"));
  entry.score_text_entity->bind();

  update_entry_text(entry);
}

inline void rebuild_entries() {
  clear_entries();
  active_rows = current_recipe.size();
  entries.reserve(current_recipe.size());
  for (size_t i = 0; i < current_recipe.size(); ++i) {
    Entry entry{};
    entry.name = current_recipe[i].first;
    entry.target = current_recipe[i].second;
    entry.current = 0;
    create_entry_visuals(entry, i);
    entry_index[entry.name] = entries.size();
    entries.push_back(entry);
  }
}

inline void init_with_targets(const std::vector<std::pair<std::string, int>>& recipe,
                              std::string task_word = "") {
  objective_word = std::move(task_word);
  current_recipe = recipe;
  if (current_recipe.empty()) {
    clear_entries();
    hide_panel();
    return;
  }
  ensure_panel();
  rebuild_entries();
  apply_layout();
}

inline void update_score(const std::string& name, int new_score, int target) {
  auto it = entry_index.find(name);
  if (it == entry_index.end()) return;
  auto& entry = entries[it->second];
  entry.current = new_score;
  entry.target = target;
  update_entry_text(entry);
}

inline void set_layout(LayoutState state) {
  layout_state = state;
  animation_active = false;
  animation_elapsed = 0.0f;
  animation_duration = 0.0f;
  current_panel_center_norm = target_center_norm(state);
  if (state == LayoutState::Hidden) {
    clear_entries();
    hide_panel();
    return;
  }
  apply_layout();
}

inline void animate_to_layout(LayoutState state, float duration = config.default_move_duration) {
  layout_state = state;
  animation_start_norm = panel_center_norm();
  animation_target_norm = target_center_norm(state);
  animation_elapsed = 0.0f;
  animation_duration = std::max(0.0f, duration);
  animation_active = animation_duration > 0.0f;
  if (!animation_active) {
    current_panel_center_norm = animation_target_norm;
  }
  apply_layout();
}

inline void start_intro_move_to_corner(float duration = 2.3f) {
  if (current_recipe.empty()) return;
  set_layout(LayoutState::CenterIntro);
  animate_to_layout(LayoutState::Corner, duration);
}

inline void start_center_shake(float duration = config.shake_duration,
                               float amplitude_px = config.shake_amplitude_px) {
  if (current_recipe.empty()) return;
  set_layout(LayoutState::CenterIntro);
  shake_elapsed = 0.0f;
  shake_duration = std::max(0.0f, duration);
  active_shake_amplitude_px = amplitude_px;
  shake_offset_px = glm::vec2{0.0f, 0.0f};
  apply_layout();
}

inline bool is_animating() {
  return animation_active || shake_duration > 0.0f;
}

struct ScoreboardController : public dynamic::DynamicObject {
  ScoreboardController() : dynamic::DynamicObject() {}
  ~ScoreboardController() override { Component::component_count--; }

  void update() override {
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    bool dirty = false;

    if (animation_active) {
      animation_elapsed += dt;
      const float t =
          animation_duration > 0.0f ? clamp01(animation_elapsed / animation_duration) : 1.0f;
      const float eased = ease_in_out(t);
      current_panel_center_norm =
          animation_start_norm + (animation_target_norm - animation_start_norm) * eased;
      if (t >= 1.0f) {
        animation_active = false;
        current_panel_center_norm = animation_target_norm;
      }
      dirty = true;
    }

    if (shake_duration > 0.0f) {
      shake_elapsed += dt;
      const float t = clamp01(shake_elapsed / shake_duration);
      const float strength = 1.0f - t;
      const float time = static_cast<float>(ecs::context().time_seconds);
      shake_offset_px = glm::vec2{
          std::sin(time * config.shake_frequency) * active_shake_amplitude_px * strength,
          std::sin(time * config.shake_frequency * 1.37f) * active_shake_amplitude_px * 0.45f *
              strength,
      };
      if (t >= 1.0f) {
        shake_duration = 0.0f;
        shake_offset_px = glm::vec2{0.0f, 0.0f};
      }
      dirty = true;
    }

    if (dirty) {
      apply_layout();
    }
  }
};

inline ScoreboardController controller{};

inline void init() {
  clear_entries();
  current_recipe.clear();
  objective_word.clear();
  layout_state = LayoutState::Corner;
  current_panel_center_norm = corner_panel_center_norm();
  animation_active = false;
  shake_duration = 0.0f;
  active_shake_amplitude_px = config.shake_amplitude_px;
  shake_offset_px = glm::vec2{0.0f, 0.0f};
  reset_panel();
}

}  // namespace scoreboard
