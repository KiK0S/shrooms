#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>

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
#include "player.hpp"
#include "shrooms_screen.hpp"

namespace tutorial {

enum class Stage {
  None,
  MoveLeft,
  MoveRight,
  CollectOne,
  ShootOne,
  PlaceFamiliarRight,
  DualCatch,
  Complete,
};

inline bool active = false;
inline Stage stage = Stage::None;
inline std::string mushroom_type = "mukhomor";

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
inline bool stage_a_done = false;
inline bool stage_b_done = false;
inline int dual_player_catches = 0;
inline int dual_familiar_catches = 0;

inline constexpr int kKeyE = 'E';

inline float view_width() { return static_cast<float>(shrooms::screen::view_width); }
inline float view_height() { return static_cast<float>(shrooms::screen::view_height); }

inline float player_center_x() {
  if (!player::player_transform) return view_width() * 0.5f;
  return player::player_transform->pos.x + player::player_size.x * 0.5f;
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
  text_obj->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, font_px);
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
  stage_a_done = false;
  stage_b_done = false;
  dual_player_catches = 0;
  dual_familiar_catches = 0;
}

inline player::FamiliarLogic* right_planted_familiar(float min_x) {
  player::FamiliarLogic* best = nullptr;
  for (auto* logic : player::familiar_logic) {
    if (!logic) continue;
    if (logic->state != player::FamiliarState::Planted) continue;
    const float x = logic->current_center().x;
    if (x < min_x) continue;
    if (!best || x > best->current_center().x) {
      best = logic;
    }
  }
  return best;
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

inline void fallback_to_place_stage(const std::string& reason) {
  set_stage(Stage::PlaceFamiliarRight, reason);
}

inline ecs::Entity* spawn_single(float x_px) {
  return levels::spawn_mushroom_now(
      mushroom_type, glm::vec2{x_px, view_height() * 0.15f});
}

inline void enter_collect_stage() {
  clear_stage_entities();
  stage_entity_a = spawn_single(std::clamp(player_center_x(), view_width() * 0.22f,
                                           view_width() * 0.78f));
}

inline void enter_shoot_stage() {
  clear_stage_entities();
  player::reset_familiars();
  stage_entity_a = spawn_single(std::clamp(player_center_x(), view_width() * 0.25f,
                                           view_width() * 0.75f));
}

inline void enter_place_familiar_stage() {
  clear_stage_entities();
  player::reset_familiars();
}

inline void enter_dual_stage() {
  clear_stage_entities();
  const float left_x = view_width() * 0.28f;
  float right_x = view_width() * 0.74f;
  if (auto* planted = right_planted_familiar(view_width() * 0.62f)) {
    right_x = planted->current_center().x;
  }
  stage_entity_a = spawn_single(left_x);
  stage_entity_b = spawn_single(right_x);
}

inline void set_stage(Stage next, const std::string& feedback) {
  stage = next;
  hide_marker();

  const std::string base_feedback = feedback.empty() ? "" : ("  " + feedback);
  switch (stage) {
    case Stage::MoveLeft: {
      clear_stage_entities();
      update_line(title_text, title_transform, "Tutorial: Stage 1/6", 24.0f, glm::vec2{0.0f, 0.82f});
      update_line(hint_text, hint_transform, "Move left with A to the glowing marker." + base_feedback,
                  18.0f, glm::vec2{0.0f, 0.72f});
      show_marker(glm::vec2{view_width() * 0.20f, view_height() * 0.82f},
                  engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
      break;
    }
    case Stage::MoveRight: {
      clear_stage_entities();
      update_line(title_text, title_transform, "Tutorial: Stage 2/6", 24.0f, glm::vec2{0.0f, 0.82f});
      update_line(hint_text, hint_transform, "Move right with D to the glowing marker." + base_feedback,
                  18.0f, glm::vec2{0.0f, 0.72f});
      show_marker(glm::vec2{view_width() * 0.80f, view_height() * 0.82f},
                  engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
      break;
    }
    case Stage::CollectOne: {
      update_line(title_text, title_transform, "Tutorial: Stage 3/6", 24.0f, glm::vec2{0.0f, 0.82f});
      update_line(hint_text, hint_transform,
                  "Catch the falling mushroom. If it falls, this stage restarts." + base_feedback,
                  18.0f, glm::vec2{0.0f, 0.72f});
      enter_collect_stage();
      break;
    }
    case Stage::ShootOne: {
      update_line(title_text, title_transform, "Tutorial: Stage 4/6", 24.0f, glm::vec2{0.0f, 0.82f});
      update_line(hint_text, hint_transform,
                  "Use W to shoot the mushroom with a familiar. Catching or missing restarts this stage." +
                      base_feedback,
                  18.0f, glm::vec2{0.0f, 0.72f});
      enter_shoot_stage();
      break;
    }
    case Stage::PlaceFamiliarRight: {
      update_line(title_text, title_transform, "Tutorial: Stage 5/6", 24.0f, glm::vec2{0.0f, 0.82f});
      update_line(hint_text, hint_transform,
                  "Press E to plant a familiar on the right side marker." + base_feedback, 18.0f,
                  glm::vec2{0.0f, 0.72f});
      show_marker(glm::vec2{view_width() * 0.76f, view_height() * 0.72f},
                  engine::UIColor{0.55f, 0.72f, 1.0f, 0.9f});
      enter_place_familiar_stage();
      break;
    }
    case Stage::DualCatch: {
      update_line(title_text, title_transform, "Tutorial: Stage 6/6", 24.0f, glm::vec2{0.0f, 0.82f});
      update_line(hint_text, hint_transform,
                  "Catch both mushrooms: one by familiar and one by player." + base_feedback, 18.0f,
                  glm::vec2{0.0f, 0.72f});
      enter_dual_stage();
      break;
    }
    case Stage::Complete: {
      clear_stage_entities(false);
      update_line(title_text, title_transform, "Tutorial Complete", 24.0f, glm::vec2{0.0f, 0.82f});
      update_line(hint_text, hint_transform, "Great. Entering victory screen...", 18.0f,
                  glm::vec2{0.0f, 0.72f});
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
  mushroom_type = levels::default_spawn_type();
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

inline void on_mushroom_caught(const std::string&, ecs::Entity* entity, bool from_familiar) {
  if (!active || !entity) return;
  if (stage == Stage::CollectOne) {
    if (is_stage_entity(entity, stage_entity_a)) {
      set_stage(Stage::ShootOne);
    }
    return;
  }
  if (stage == Stage::ShootOne) {
    if (is_stage_entity(entity, stage_entity_a)) {
      restart_stage("That one must be shot.");
    }
    return;
  }
  if (stage == Stage::DualCatch) {
    if (is_stage_entity(entity, stage_entity_a) && !stage_a_done) {
      stage_a_done = true;
      if (from_familiar) {
        dual_familiar_catches += 1;
      } else {
        dual_player_catches += 1;
      }
    } else if (is_stage_entity(entity, stage_entity_b) && !stage_b_done) {
      stage_b_done = true;
      if (from_familiar) {
        dual_familiar_catches += 1;
      } else {
        dual_player_catches += 1;
      }
    }

    if (stage_a_done && stage_b_done) {
      if (dual_familiar_catches >= 1 && dual_player_catches >= 1) {
        set_stage(Stage::Complete);
      } else {
        fallback_to_place_stage("Use both player and familiar catches.");
      }
    }
  }
}

inline void on_mushroom_missed(const std::string&, ecs::Entity* entity) {
  if (!active || !entity) return;
  if (stage == Stage::CollectOne && is_stage_entity(entity, stage_entity_a)) {
    restart_stage("It fell. Try again.");
    return;
  }
  if (stage == Stage::ShootOne && is_stage_entity(entity, stage_entity_a)) {
    restart_stage("It fell. Shoot it before it drops.");
    return;
  }
  if (stage == Stage::DualCatch &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b))) {
    fallback_to_place_stage("A mushroom fell. Re-place familiar and retry.");
  }
}

inline void on_mushroom_sorted(const std::string&, ecs::Entity* entity) {
  if (!active || !entity) return;
  if (stage == Stage::CollectOne && is_stage_entity(entity, stage_entity_a)) {
    restart_stage("This stage needs a catch, not a shot.");
    return;
  }
  if (stage == Stage::ShootOne && is_stage_entity(entity, stage_entity_a)) {
    set_stage(Stage::PlaceFamiliarRight);
    return;
  }
  if (stage == Stage::DualCatch &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b))) {
    fallback_to_place_stage("Do not shoot in this stage.");
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
    if (stage == Stage::PlaceFamiliarRight) {
      if (right_planted_familiar(view_width() * 0.62f)) {
        set_stage(Stage::DualCatch);
        return;
      }
      if (any_planted_familiar()) {
        restart_stage("Place it in the right zone.");
        return;
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
