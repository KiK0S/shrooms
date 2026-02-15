#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
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
  float text_font_px = 20.0f;
  glm::vec4 text_color = glm::vec4(1.0f);
  int layer = 1;
} config;

struct Entry {
  std::string name;
  int current = 0;
  int target = 0;
  ecs::Entity* icon = nullptr;
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
inline size_t active_rows = 0;

inline glm::vec2 panel_reference_size() {
  const glm::vec2 measured = shrooms::texture_sizing::reference_size("menu_scoreboard");
  if (measured.x > 0.0f && measured.y > 0.0f) return measured;
  return shrooms::texture_sizing::reference_size_from_width("menu_scoreboard", 64.0f);
}

inline glm::vec2 panel_size_px() {
  return shrooms::texture_sizing::from_reference_size(panel_reference_size());
}

inline glm::vec2 panel_center_norm() {
  const glm::vec2 size = panel_size_px();
  return glm::vec2{
      1.0f - (size.x / static_cast<float>(shrooms::screen::view_width)),
      1.0f - (size.y / static_cast<float>(shrooms::screen::view_height)),
  };
}

inline float row_center_y_norm(size_t row_index) {
  const float panel_center_y = panel_center_norm().y;
  if (active_rows == 0) return panel_center_y;
  const float middle = static_cast<float>(active_rows - 1) * 0.5f;
  return panel_center_y + (static_cast<float>(row_index) - middle) * config.row_spacing;
}

inline glm::vec2 row_icon_position(size_t row_index) {
  return shrooms::screen::norm_to_pixels(glm::vec2{config.icon_x, row_center_y_norm(row_index)});
}

inline glm::vec2 row_score_center(size_t row_index) {
  return shrooms::screen::norm_to_pixels(glm::vec2{config.score_x, row_center_y_norm(row_index)});
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
  const glm::vec2 size = panel_size_px();
  const glm::vec2 center = shrooms::screen::norm_to_pixels(panel_center_norm());
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = shrooms::screen::center_to_top_left(center, size);
  panel->add(transform);
  panel->add(arena::create<layers::ConstLayer>(config.layer));
  const engine::TextureId tex_id = engine::resources::register_texture("menu_scoreboard");
  panel->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  panel->add(arena::create<scene::SceneObject>("main"));
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
  rebuild_entries();
}

inline void update_score(const std::string& name, int new_score, int target) {
  auto it = entry_index.find(name);
  if (it == entry_index.end()) return;
  auto& entry = entries[it->second];
  entry.current = new_score;
  entry.target = target;
  update_entry_text(entry);
}

inline void init() {
  clear_entries();
  current_recipe.clear();
  objective_word.clear();
  reset_panel();
}

}  // namespace scoreboard
