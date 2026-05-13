#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "level_manager.hpp"
#include "controls.hpp"
#include "player.hpp"
#include "shrooms_screen.hpp"

namespace tutorial {

enum class Stage {
  None,
  MoveLeft,
  MoveRight,
  CollectOne,
  PlaceTrap,
  CollectorComplete,
  RecipeIntro,
  ShootTarget,
  RecipeScenario,
  Complete,
};

inline bool active = false;
inline Stage stage = Stage::None;
inline std::string good_mushroom_type = "borovik";
inline std::string bad_mushroom_type = "mukhomor";
inline float stage_timer = 0.0f;

inline ecs::Entity* title_entity = nullptr;
inline text::TextObject* title_text = nullptr;
inline transform::NoRotationTransform* title_transform = nullptr;
inline hidden::HiddenObject* title_hidden = nullptr;

inline ecs::Entity* hint_entity = nullptr;
inline text::TextObject* hint_text = nullptr;
inline transform::NoRotationTransform* hint_transform = nullptr;
inline hidden::HiddenObject* hint_hidden = nullptr;
inline color::OneColor* hint_color = nullptr;

inline ecs::Entity* marker_entity = nullptr;
inline transform::NoRotationTransform* marker_transform = nullptr;
inline hidden::HiddenObject* marker_hidden = nullptr;
inline render_system::CircleRenderable* marker_circle = nullptr;
inline float marker_radius = 0.0f;

inline ecs::Entity* stage_entity_a = nullptr;
inline ecs::Entity* stage_entity_b = nullptr;
inline ecs::Entity* stage_entity_c = nullptr;
inline bool stage_a_done = false;
inline bool stage_b_done = false;
inline bool stage_c_done = false;
inline bool recipe_bad_removed = false;
inline int recipe_good_catches = 0;

inline constexpr glm::vec2 kTitleCenterNorm = glm::vec2{0.0f, 0.16f};
inline constexpr glm::vec2 kHintCenterNorm = glm::vec2{0.0f, -0.02f};
inline constexpr float kMaxTextWidthRatio = 0.86f;

inline float view_width() { return static_cast<float>(shrooms::screen::view_width); }
inline float view_height() { return static_cast<float>(shrooms::screen::view_height); }

inline float title_font_px() { return controls::is_mobile_layout() ? 30.0f : 26.0f; }

inline float hint_font_px() { return controls::is_mobile_layout() ? 23.0f : 20.0f; }

inline float player_center_x() {
  if (!player::player_transform) return view_width() * 0.5f;
  return player::player_transform->pos.x + player::player_size.x * 0.5f;
}

inline std::string trim_copy(const std::string& value) {
  size_t first = 0;
  while (first < value.size() &&
         std::isspace(static_cast<unsigned char>(value[first])) != 0) {
    ++first;
  }
  size_t last = value.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(value[last - 1])) != 0) {
    --last;
  }
  return value.substr(first, last - first);
}

inline bool is_alpha_char(char ch) {
  return std::isalpha(static_cast<unsigned char>(ch)) != 0;
}

inline int break_priority(char ch) {
  switch (ch) {
    case '.':
      return 5;
    case '!':
    case '?':
      return 4;
    case ';':
    case ':':
      return 3;
    case ',':
      return 2;
    default:
      return is_alpha_char(ch) ? -1 : 1;
  }
}

inline float text_width_px(const std::string& value, float font_px) {
  return engine::text::layout_text(value, 0.0f, 0.0f, font_px).width;
}

inline std::string join_lines(const std::vector<std::string>& lines) {
  if (lines.empty()) return "";
  std::string out = lines.front();
  for (size_t i = 1; i < lines.size(); ++i) {
    out += '\n';
    out += lines[i];
  }
  return out;
}

inline std::string wrap_text_for_view(const std::string& value, float font_px,
                                      float max_width_px) {
  if (value.empty()) return value;
  if (text_width_px(value, font_px) <= max_width_px) return value;

  std::vector<std::string> lines{};
  size_t start = 0;
  while (start < value.size()) {
    while (start < value.size() &&
           std::isspace(static_cast<unsigned char>(value[start])) != 0) {
      ++start;
    }
    if (start >= value.size()) break;

    const std::string remaining = value.substr(start);
    if (text_width_px(remaining, font_px) <= max_width_px) {
      const std::string tail = trim_copy(remaining);
      if (!tail.empty()) lines.push_back(tail);
      break;
    }

    size_t best_break = std::string::npos;
    int best_priority = -1;
    for (size_t i = start; i < value.size(); ++i) {
      const char ch = value[i];
      if (is_alpha_char(ch)) continue;
      const std::string candidate = value.substr(start, i - start + 1);
      if (text_width_px(candidate, font_px) > max_width_px) break;
      const int priority = break_priority(ch);
      if (priority > best_priority ||
          (priority == best_priority &&
           (best_break == std::string::npos || i > best_break))) {
        best_break = i;
        best_priority = priority;
      }
    }

    if (best_break == std::string::npos) {
      size_t hard_break = start;
      for (size_t i = start; i < value.size(); ++i) {
        const std::string candidate = value.substr(start, i - start + 1);
        if (text_width_px(candidate, font_px) > max_width_px) break;
        hard_break = i;
      }
      if (hard_break == start && start + 1 < value.size()) {
        hard_break = start + 1;
      }
      best_break = hard_break;
    }

    const std::string line = trim_copy(value.substr(start, best_break - start + 1));
    if (!line.empty()) lines.push_back(line);
    start = best_break + 1;
  }

  if (lines.empty()) return value;
  return join_lines(lines);
}

inline void set_visible(bool visible) {
  if (title_hidden) {
    title_hidden->set_visible(visible);
  }
  if (hint_hidden) {
    hint_hidden->set_visible(visible);
  }
  if (marker_hidden) {
    marker_hidden->set_visible(false);
  }
}

inline void update_line(text::TextObject* text_obj, transform::NoRotationTransform* transform,
                        const std::string& value, float font_px, const glm::vec2& center_norm) {
  if (!text_obj || !transform) return;
  const std::string wrapped =
      wrap_text_for_view(value, font_px, view_width() * kMaxTextWidthRatio);
  text_obj->text = wrapped;
  const auto layout = engine::text::layout_text(wrapped, 0.0f, 0.0f, font_px);
  const glm::vec2 size{layout.width, layout.height};
  const glm::vec2 center = shrooms::screen::norm_to_pixels(center_norm);
  transform->pos = shrooms::screen::center_to_top_left(center, size);
}

inline void set_marker_center(const glm::vec2& center_px) {
  if (!marker_transform) return;
  const glm::vec2 size{marker_radius * 2.0f, marker_radius * 2.0f};
  marker_transform->pos = shrooms::screen::center_to_top_left(center_px, size);
}

inline void show_marker(const glm::vec2& center_px, const engine::UIColor& color) {
  if (!marker_hidden || !marker_circle) return;
  marker_hidden->show();
  marker_circle->color = color;
  set_marker_center(center_px);
}

inline void hide_marker() {
  if (!marker_hidden) return;
  marker_hidden->hide();
}

inline void clear_stage_entities(bool delete_entities = true) {
  auto clear_one = [&](ecs::Entity*& entity) {
    if (!entity) return;
    if (delete_entities && !entity->is_pending_deletion()) {
      entity->mark_deleted();
    }
    entity = nullptr;
  };
  clear_one(stage_entity_a);
  clear_one(stage_entity_b);
  clear_one(stage_entity_c);
  stage_a_done = false;
  stage_b_done = false;
  stage_c_done = false;
  recipe_bad_removed = false;
  recipe_good_catches = 0;
}

inline bool any_planted_familiar() {
  for (auto* logic : player::familiar_logic) {
    if (!logic) continue;
    if (logic->state == player::FamiliarState::Planted) {
      return true;
    }
  }
  return false;
}

inline void set_stage(Stage next, const std::string& feedback = "");

inline void restart_stage(const std::string& reason) {
  set_stage(stage, reason);
}

inline ecs::Entity* spawn_single(const std::string& type, float x_px, float y_norm = 0.15f) {
  return levels::spawn_mushroom_now(
      type, glm::vec2{x_px, view_height() * y_norm});
}

inline void enter_collect_stage() {
  clear_stage_entities();
  stage_entity_a = spawn_single(good_mushroom_type,
                                std::clamp(player_center_x(), view_width() * 0.22f,
                                           view_width() * 0.78f));
}

inline void enter_place_trap_stage() {
  clear_stage_entities();
  player::reset_familiars();
}

inline void enter_shoot_target_stage() {
  clear_stage_entities();
  player::reset_familiars();
  const glm::vec2 center{view_width() * 0.5f, view_height() * 0.15f};
  show_marker(center, engine::UIColor{0.95f, 0.35f, 0.35f, 0.9f});
  stage_entity_a = levels::spawn_mushroom_now(bad_mushroom_type, center);
}

inline void enter_recipe_scenario_stage() {
  clear_stage_entities();
  player::reset_familiars();
  const glm::vec2 bad_center{view_width() * 0.5f, view_height() * 0.14f};
  show_marker(bad_center, engine::UIColor{0.95f, 0.35f, 0.35f, 0.9f});
  stage_entity_a = levels::spawn_mushroom_now(bad_mushroom_type, bad_center);
  stage_entity_b = spawn_single(good_mushroom_type, view_width() * 0.28f, 0.12f);
  stage_entity_c = spawn_single(good_mushroom_type, view_width() * 0.72f, 0.12f);
}

inline void maybe_complete_recipe_scenario() {
  if (recipe_bad_removed && recipe_good_catches >= 2) {
    set_stage(Stage::Complete);
  }
}

inline void set_stage(Stage next, const std::string& feedback) {
  stage = next;
  stage_timer = 0.0f;
  hide_marker();

  const std::string base_feedback = feedback.empty() ? "" : ("  " + feedback);
  switch (stage) {
    case Stage::MoveLeft: {
      clear_stage_entities();
      update_line(title_text, title_transform, "Tutorial: Stage 1/8", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Move left with " + controls::bound_key_label(controls::Action::MoveLeft) +
                      " to the glowing marker." + base_feedback,
                  hint_font_px(), kHintCenterNorm);
      show_marker(glm::vec2{view_width() * 0.20f, view_height() * 0.82f},
                  engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
      break;
    }
    case Stage::MoveRight: {
      clear_stage_entities();
      update_line(title_text, title_transform, "Tutorial: Stage 2/8", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Move right with " + controls::bound_key_label(controls::Action::MoveRight) +
                      " to the glowing marker." + base_feedback,
                  hint_font_px(), kHintCenterNorm);
      show_marker(glm::vec2{view_width() * 0.80f, view_height() * 0.82f},
                  engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
      break;
    }
    case Stage::CollectOne: {
      update_line(title_text, title_transform, "Tutorial: Stage 3/8", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Collector mode starts here: catch the falling mushroom. If it falls, you lose a life." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      enter_collect_stage();
      break;
    }
    case Stage::PlaceTrap: {
      update_line(title_text, title_transform, "Tutorial: Stage 4/8", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Place a trap with " + controls::bound_key_label(controls::Action::Trap) +
                      ". Collector mode never needs shooting." + base_feedback,
                  hint_font_px(), kHintCenterNorm);
      show_marker(glm::vec2{view_width() * 0.62f, view_height() * 0.72f},
                  engine::UIColor{0.55f, 0.72f, 1.0f, 0.9f});
      enter_place_trap_stage();
      break;
    }
    case Stage::CollectorComplete: {
      clear_stage_entities();
      update_line(title_text, title_transform, "Collector Mode", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "That is enough for Collector: catch mushrooms, use traps, protect your 3 lives." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      break;
    }
    case Stage::RecipeIntro: {
      clear_stage_entities();
      update_line(title_text, title_transform, "Recipe Mode", title_font_px(), kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Recipe mode shows icon counts before each level. Any mushroom hitting the floor ends the run." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      break;
    }
    case Stage::ShootTarget: {
      update_line(title_text, title_transform, "Tutorial: Stage 7/8", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Use " + controls::bound_key_label(controls::Action::Shoot) +
                      " to shoot the marked wrong mushroom before it reaches the basket." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      enter_shoot_target_stage();
      break;
    }
    case Stage::RecipeScenario: {
      update_line(title_text, title_transform, "Tutorial: Stage 8/8", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Keep the basket white-only: catch the borovik mushrooms and shoot the center mukhomor." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      enter_recipe_scenario_stage();
      break;
    }
    case Stage::Complete: {
      clear_stage_entities(false);
      update_line(title_text, title_transform, "Tutorial Complete", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Returning to the menu with Collector selected.", hint_font_px(),
                  kHintCenterNorm);
      levels::finish_tutorial(true);
      break;
    }
    case Stage::None:
    default:
      clear_stage_entities();
      break;
  }
}

inline void start() {
  active = true;
  good_mushroom_type = "borovik";
  bad_mushroom_type = "mukhomor";
  set_visible(true);
  set_stage(Stage::MoveLeft);
}

inline void stop() {
  active = false;
  stage = Stage::None;
  clear_stage_entities();
  set_visible(false);
}

inline bool is_active() { return active; }

inline bool is_stage_entity(ecs::Entity* entity, ecs::Entity* tracked) {
  if (!entity || !tracked) return false;
  return entity == tracked;
}

inline void on_mushroom_spawned(const std::string&, ecs::Entity*) {}

inline void on_mushroom_caught(const std::string&, ecs::Entity* entity, bool) {
  if (!active || !entity) return;
  if (stage == Stage::CollectOne) {
    if (is_stage_entity(entity, stage_entity_a)) {
      set_stage(Stage::PlaceTrap);
    }
    return;
  }
  if (stage == Stage::ShootTarget) {
    if (is_stage_entity(entity, stage_entity_a)) {
      restart_stage("Shoot the marked mushroom instead of catching it.");
    }
    return;
  }
  if (stage == Stage::RecipeScenario) {
    if (is_stage_entity(entity, stage_entity_a)) {
      restart_stage("The mukhomor must be shot, not caught.");
      return;
    }
    if (is_stage_entity(entity, stage_entity_b) && !stage_b_done) {
      stage_b_done = true;
      recipe_good_catches += 1;
    } else if (is_stage_entity(entity, stage_entity_c) && !stage_c_done) {
      stage_c_done = true;
      recipe_good_catches += 1;
    }
    maybe_complete_recipe_scenario();
  }
}

inline void on_mushroom_missed(const std::string&, ecs::Entity* entity) {
  if (!active || !entity) return;
  if (stage == Stage::CollectOne && is_stage_entity(entity, stage_entity_a)) {
    restart_stage("It fell. Try again.");
    return;
  }
  if (stage == Stage::ShootTarget && is_stage_entity(entity, stage_entity_a)) {
    restart_stage("It fell. Shoot it before it drops.");
    return;
  }
  if (stage == Stage::RecipeScenario &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b) ||
       is_stage_entity(entity, stage_entity_c))) {
    restart_stage("Recipe mode ends if anything hits the floor.");
  }
}

inline void on_mushroom_sorted(const std::string&, ecs::Entity* entity) {
  if (!active || !entity) return;
  if (stage == Stage::CollectOne && is_stage_entity(entity, stage_entity_a)) {
    restart_stage("This stage needs a catch, not a shot.");
    return;
  }
  if (stage == Stage::ShootTarget && is_stage_entity(entity, stage_entity_a)) {
    set_stage(Stage::RecipeScenario);
    return;
  }
  if (stage == Stage::RecipeScenario) {
    if (is_stage_entity(entity, stage_entity_a)) {
      recipe_bad_removed = true;
      stage_a_done = true;
      maybe_complete_recipe_scenario();
      return;
    }
    if (is_stage_entity(entity, stage_entity_b) || is_stage_entity(entity, stage_entity_c)) {
      restart_stage("Borovik belongs in the basket.");
    }
  }
}

struct TutorialController : public dynamic::DynamicObject {
  TutorialController() : dynamic::DynamicObject() {}
  ~TutorialController() override { Component::component_count--; }

  void update() override {
    if (!active) return;
    if (!levels::is_tutorial_mode()) {
      stop();
      return;
    }
    if (scene::is_current_scene_paused()) return;
    stage_timer += static_cast<float>(ecs::context().delta_seconds);

    if (stage == Stage::MoveLeft) {
      if (player_center_x() <= view_width() * 0.22f) {
        set_stage(Stage::MoveRight);
      }
      return;
    }
    if (stage == Stage::MoveRight) {
      if (player_center_x() >= view_width() * 0.78f) {
        set_stage(Stage::CollectOne);
      }
      return;
    }
    if (stage == Stage::PlaceTrap) {
      if (any_planted_familiar()) {
        set_stage(Stage::CollectorComplete);
        return;
      }
      return;
    }
    if (stage == Stage::CollectorComplete) {
      if (stage_timer >= 2.7f) {
        set_stage(Stage::RecipeIntro);
      }
      return;
    }
    if (stage == Stage::RecipeIntro) {
      if (stage_timer >= 3.6f) {
        set_stage(Stage::ShootTarget);
      }
      return;
    }
  }
};

inline TutorialController tutorial_controller{};

inline void init() {
  levels::set_tutorial_hooks(
      [](const std::string& type, ecs::Entity* entity) { on_mushroom_spawned(type, entity); },
      [](const std::string& type, ecs::Entity* entity, bool from_familiar) {
        on_mushroom_caught(type, entity, from_familiar);
      },
      [](const std::string& type, ecs::Entity* entity) { on_mushroom_missed(type, entity); },
      [](const std::string& type, ecs::Entity* entity) { on_mushroom_sorted(type, entity); });

  title_entity = arena::create<ecs::Entity>();
  title_transform = arena::create<transform::NoRotationTransform>();
  title_entity->add(title_transform);
  title_entity->add(arena::create<layers::ConstLayer>(9));
  title_text = arena::create<text::TextObject>("", 24.0f);
  title_entity->add(title_text);
  title_hidden = arena::create<hidden::HiddenObject>();
  title_entity->add(title_hidden);
  title_entity->add(arena::create<scene::SceneObject>("main"));

  hint_entity = arena::create<ecs::Entity>();
  hint_transform = arena::create<transform::NoRotationTransform>();
  hint_entity->add(hint_transform);
  hint_entity->add(arena::create<layers::ConstLayer>(9));
  hint_text = arena::create<text::TextObject>("", 18.0f);
  hint_entity->add(hint_text);
  hint_color = arena::create<color::OneColor>(glm::vec4{0.95f, 0.95f, 0.95f, 1.0f});
  hint_entity->add(hint_color);
  hint_hidden = arena::create<hidden::HiddenObject>();
  hint_entity->add(hint_hidden);
  hint_entity->add(arena::create<scene::SceneObject>("main"));

  marker_entity = arena::create<ecs::Entity>();
  marker_transform = arena::create<transform::NoRotationTransform>();
  marker_entity->add(marker_transform);
  marker_entity->add(arena::create<layers::ConstLayer>(9));
  marker_radius = std::max(14.0f, shrooms::screen::scale_to_pixels(glm::vec2{0.015f, 0.0f}).x);
  marker_circle = arena::create<render_system::CircleRenderable>(
      marker_radius, engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
  marker_entity->add(marker_circle);
  marker_hidden = arena::create<hidden::HiddenObject>();
  marker_entity->add(marker_hidden);
  marker_entity->add(arena::create<scene::SceneObject>("main"));

  stop();
}

}  // namespace tutorial
