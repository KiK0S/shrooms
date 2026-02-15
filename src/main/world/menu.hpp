#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <string>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/color/color_object.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/transformation/transform_object.hpp"
#include "systems/hidden/hidden_object.hpp"

#include "countdown.hpp"
#include "leaderboard.hpp"
#include "level_manager.hpp"
#include "lives.hpp"
#include "player.hpp"
#include "tutorial.hpp"
#include "camera_shake.hpp"
#include "vfx.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "shrooms_scenes.hpp"
#include "systems/text_input/text_input_system.hpp"

namespace menu {

constexpr size_t kMaxLevelLines = 10;
constexpr float kMenuTextX = -0.8f;
constexpr float kLeaderboardTextX = 0.15f;
constexpr size_t kLeaderboardLines = static_cast<size_t>(leaderboard::kMaxEntries);
constexpr size_t kNameMaxLength = 12;
constexpr int kKeyArrowUpDom = 38;
constexpr int kKeyArrowDownDom = 40;
constexpr int kKeyArrowUpSdl = 1073741906;
constexpr int kKeyArrowDownSdl = 1073741905;

struct LineTextColor : public color::ColoredObject {
  explicit LineTextColor(const glm::vec4& value) : color::ColoredObject(), value(value) {}
  ~LineTextColor() override { Component::component_count--; }
  glm::vec4 get_color() override { return value; }
  glm::vec4 value{1.0f, 1.0f, 1.0f, 1.0f};
};

struct TextLine {
  ecs::Entity* entity = nullptr;
  transform::NoRotationTransform* transform = nullptr;
  text::TextObject* text_object = nullptr;
  hidden::HiddenObject* text_hidden = nullptr;
  LineTextColor* text_color = nullptr;
  ecs::Entity* button_entity = nullptr;
  transform::NoRotationTransform* button_transform = nullptr;
  render_system::QuadRenderable* button_quad = nullptr;
  hidden::HiddenObject* button_hidden = nullptr;
  glm::vec2 size{0.0f, 0.0f};
  glm::vec2 button_size{0.0f, 0.0f};
  glm::vec2 button_base_pos{0.0f, 0.0f};
  glm::vec2 button_base_size{0.0f, 0.0f};
  engine::UIColor base_button_color{0.0f, 0.0f, 0.0f, 0.0f};
  engine::UIColor hover_button_color{0.2f, 0.15f, 0.3f, 0.85f};
  engine::UIColor selected_button_color{0.15f, 0.15f, 0.15f, 0.95f};
  engine::UIColor dimmed_button_color{0.1f, 0.1f, 0.1f, 0.5f};
  glm::vec4 base_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 selected_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 dimmed_text_color{0.74f, 0.74f, 0.74f, 0.92f};
  float hover_scale = 1.04f;
  float selected_scale = 1.0f;
  float font_px = 18.0f;
};

inline TextLine status_line{};
inline TextLine instruction_line{};
inline TextLine difficulty_line{};
inline TextLine tutorial_line{};
inline TextLine credits_line{};
inline std::array<TextLine, kMaxLevelLines> level_lines{};
inline size_t active_level_lines = 0;

inline ecs::Entity* character_entity = nullptr;
inline ecs::Entity* menu_background = nullptr;

enum class MenuMode {
  Main,
  Objective,
  GameOver,
};

inline MenuMode menu_mode = MenuMode::Main;

inline TextLine objective_title{};
inline TextLine objective_level{};
inline TextLine objective_hint{};
inline std::array<TextLine, kMaxLevelLines> objective_recipe_lines{};
inline size_t active_objective_lines = 0;

inline TextLine gameover_title{};
inline TextLine gameover_level{};
inline TextLine gameover_collected{};
inline TextLine gameover_sorted{};
inline TextLine gameover_hint{};
inline TextLine gameover_name_prompt{};
inline TextLine gameover_name_value{};
inline TextLine gameover_restart{};
inline TextLine gameover_main_menu{};
inline TextLine leaderboard_title{};
inline std::array<TextLine, kLeaderboardLines> leaderboard_lines{};

inline size_t pending_level_index = 0;
inline bool has_pending_level = false;
inline bool pending_infinite = false;
inline bool pending_tutorial = false;
inline bool awaiting_name_entry = false;
inline bool show_leaderboard = false;
inline int pending_leaderboard_score = 0;
inline int block_input_frames = 0;

inline void suppress_input_for_frames(int frames = 2) {
  block_input_frames = std::max(block_input_frames, frames);
}

inline void enter_objective_mode(size_t level_index);
inline void enter_infinite_objective_mode();
inline void enter_tutorial_objective_mode();
inline void enter_game_over_mode();
inline void enter_main_menu_mode();

inline void update_text(TextLine& line, const std::string& value) {
  if (!line.text_object) return;
  line.text_object->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, line.font_px);
  line.size = glm::vec2{layout.width, layout.height};
  if (line.button_transform && line.button_quad) {
    const float pad_x = 14.0f;
    const float pad_y = 8.0f;
    line.button_size = glm::vec2{line.size.x + pad_x * 2.0f, line.size.y + pad_y * 2.0f};
    line.button_base_size = line.button_size;
    line.button_base_pos = line.transform ? (line.transform->pos - glm::vec2{pad_x, pad_y})
                                          : glm::vec2{0.0f, 0.0f};
    line.button_transform->pos = line.button_base_pos;
    line.button_quad->width = line.button_base_size.x;
    line.button_quad->height = line.button_base_size.y;
    line.button_quad->color = line.base_button_color;
  }
}

inline void set_line_visibility(TextLine& line, bool text_visible, bool button_visible) {
  if (line.text_hidden) {
    line.text_hidden->set_visible(text_visible);
  }
  if (line.button_hidden) {
    line.button_hidden->set_visible(button_visible);
  }
}

inline void apply_button_visual(TextLine& line, float scale, const engine::UIColor& color) {
  if (!line.button_transform || !line.button_quad) return;
  const glm::vec2 size = line.button_base_size * scale;
  const glm::vec2 center = line.button_base_pos + line.button_base_size * 0.5f;
  line.button_transform->pos = center - size * 0.5f;
  line.button_quad->width = size.x;
  line.button_quad->height = size.y;
  line.button_quad->color = color;
}

inline bool is_arrow_up_key(int key) {
  return key == kKeyArrowUpDom || key == kKeyArrowUpSdl;
}

inline bool is_arrow_down_key(int key) {
  return key == kKeyArrowDownDom || key == kKeyArrowDownSdl;
}

inline bool is_confirm_key(int key) {
  return key == ' ' || key == '\r' || key == '\n' || key == 13;
}

inline void set_line_visual_state(TextLine& line, bool selected, bool hovered, bool dimmed) {
  if (line.text_color) {
    line.text_color->value = selected ? line.selected_text_color
                                      : (dimmed ? line.dimmed_text_color : line.base_text_color);
  }
  if (!line.button_quad) return;
  if (selected) {
    apply_button_visual(line, line.selected_scale, line.selected_button_color);
    return;
  }
  if (hovered) {
    apply_button_visual(line, line.hover_scale, line.hover_button_color);
    return;
  }
  if (dimmed) {
    apply_button_visual(line, 1.0f, line.dimmed_button_color);
    return;
  }
  apply_button_visual(line, 1.0f, line.base_button_color);
}

inline void set_line_hovered(TextLine& line, bool hovered) {
  set_line_visual_state(line, false, hovered, false);
}

inline void set_entity_visible(ecs::Entity* entity, bool visible) {
  if (!entity) return;
  auto* hidden = entity->get<hidden::HiddenObject>();
  if (!hidden) return;
  hidden->set_visible(visible);
}

inline bool show_infinite_entry() {
  return levels::infinite_available();
}

inline size_t available_levels() {
  size_t count = levels::parsed_levels.size();
  if (show_infinite_entry()) {
    count += 1;
  }
  return std::min(kMaxLevelLines, count);
}

inline bool is_infinite_entry(size_t index) {
  return show_infinite_entry() && index == levels::infinite_menu_index();
}

inline bool is_selectable_level(size_t index) {
  if (index >= active_level_lines) return false;
  return is_infinite_entry(index) || levels::is_unlocked(index);
}

inline std::string format_level_line(size_t index) {
  if (is_infinite_entry(index)) {
    std::string label = std::to_string(index + 1) + ". Infinite Mode";
    if (levels::last_played_level_index == index && !levels::level_finished) {
      label += "  [in progress]";
    } else if (levels::last_played_level_index == index && levels::level_finished) {
      label += "  [last]";
    }
    return label;
  }
  if (index >= levels::parsed_levels.size()) {
    return "";
  }
  const auto& definition = levels::parsed_levels[index];
  std::string label = std::to_string(index + 1) + ". " + definition.id;
  if (!levels::is_unlocked(index)) {
    label += "  [locked]";
    return label;
  }
  if (levels::last_played_level_index == index && !levels::level_finished) {
    label += "  [in progress]";
  } else if (levels::last_played_level_index == index && levels::level_finished) {
    label += "  [last]";
  }
  return label;
}

inline void refresh_status_line() {
  update_text(status_line, "Last game: " + levels::last_game_status);
}

inline void refresh_instruction_line() {
  size_t count = available_levels();
  if (count == 0) {
    update_text(instruction_line, "No levels available yet. Press T for tutorial.");
    return;
  }
  update_text(instruction_line, "Use Up/Down + Enter. Select Difficulty to switch mode.");
}

inline void refresh_difficulty_line() {
  update_text(difficulty_line, "Difficulty: " + levels::difficulty_label());
}

inline void refresh_tutorial_line() {
  update_text(tutorial_line, "Tutorial");
}

inline void refresh_level_lines() {
  active_level_lines = available_levels();
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    if (i < active_level_lines) {
      update_text(level_lines[i], format_level_line(i));
    } else {
      update_text(level_lines[i], "");
    }
  }
}

inline bool point_hits_line(const TextLine& line, const glm::vec2& point) {
  if (!line.transform && !line.button_transform) return false;
  const glm::vec2 pos = line.button_transform ? line.button_base_pos : line.transform->pos;
  const glm::vec2 size = line.button_transform ? line.button_base_size : line.size;
  if (point.x < pos.x || point.x > pos.x + size.x) return false;
  if (point.y < pos.y || point.y > pos.y + size.y) return false;
  return true;
}

inline std::optional<size_t> level_index_at_point(const glm::vec2& point) {
  size_t count = active_level_lines;
  for (size_t i = 0; i < count; ++i) {
    const auto& line = level_lines[i];
    if (point_hits_line(line, point)) return i;
  }
  return std::nullopt;
}

inline void handle_level_selection(size_t index) {
  if (is_infinite_entry(index)) {
    enter_infinite_objective_mode();
    return;
  }
  if (index >= levels::parsed_levels.size()) {
    return;
  }
  if (!levels::is_unlocked(index)) {
    return;
  }
  enter_objective_mode(index);
}

inline void refresh_objective_lines_from_level(const levels::LevelDefinition& level,
                                               const std::string& label) {
  update_text(objective_title, "Objective");
  update_text(objective_level, label);
  active_objective_lines = std::min(kMaxLevelLines, level.recipe_order.size());
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    if (i < active_objective_lines) {
      const auto& item = level.recipe_order[i];
      update_text(objective_recipe_lines[i], levels::objective_line_text(level, item.first, item.second));
    } else {
      update_text(objective_recipe_lines[i], "");
    }
  }
  update_text(objective_hint, "Press Enter or tap to start");
}

inline void refresh_objective_lines(size_t level_index) {
  if (level_index >= levels::parsed_levels.size()) {
    active_objective_lines = 0;
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      update_text(objective_recipe_lines[i], "");
    }
    return;
  }
  const auto& level = levels::parsed_levels[level_index];
  refresh_objective_lines_from_level(
      level, "Level " + std::to_string(level_index + 1) + ": " + level.id);
}

inline void refresh_leaderboard_lines() {
  if (!show_leaderboard) {
    update_text(leaderboard_title, "");
    for (auto& line : leaderboard_lines) {
      update_text(line, "");
    }
    return;
  }
  update_text(leaderboard_title, "Global Leaderboard");
  const auto& entries = leaderboard::list();
  for (size_t i = 0; i < leaderboard_lines.size(); ++i) {
    if (i < entries.size()) {
      const auto& entry = entries[i];
      const std::string line = std::to_string(i + 1) + ". " + entry.name + " - " +
                               std::to_string(entry.score);
      update_text(leaderboard_lines[i], line);
    } else {
      update_text(leaderboard_lines[i], "");
    }
  }
}

inline void refresh_name_entry_lines() {
  if (!show_leaderboard || !awaiting_name_entry) {
    update_text(gameover_name_prompt, "");
    update_text(gameover_name_value, "");
    return;
  }
  update_text(gameover_name_prompt, "New high score! Enter your name:");
  std::string display = text_input::value();
  if (display.empty()) {
    display = "_";
  } else {
    display += "_";
  }
  update_text(gameover_name_value, display);
}

inline void refresh_gameover_lines() {
  if (!levels::last_result_valid) return;
  const auto& result = levels::last_result;
  if (result.infinite_mode) {
    show_leaderboard = true;
    update_text(gameover_title, result.success ? "Run Complete" : "Run Over");
    update_text(gameover_level,
                "Infinite Mode - Round " + std::to_string(result.round_index));
    update_text(gameover_collected, "Score: " + std::to_string(result.global_score));
    update_text(gameover_sorted, "Rounds won: " + std::to_string(result.rounds_won));
    if (awaiting_name_entry) {
      update_text(gameover_hint, "Press Enter to submit your name");
    } else {
      update_text(gameover_hint, "Use Up/Down + Enter, or press R/M.");
    }
    update_text(gameover_restart, "Restart");
    update_text(gameover_main_menu, "Main Menu");
    refresh_name_entry_lines();
    refresh_leaderboard_lines();
    return;
  }
  show_leaderboard = false;
  awaiting_name_entry = false;
  if (result.tutorial_mode) {
    update_text(gameover_title, result.success ? "Tutorial Complete" : "Tutorial Failed");
    update_text(gameover_level, "Tutorial");
  } else {
    update_text(gameover_title, result.success ? "Level Complete" : "Game Over");
    update_text(gameover_level,
                "Level " + std::to_string(result.level_index + 1) + ": " + result.level_id);
  }
  update_text(gameover_collected, "Collected: " + std::to_string(result.collected));
  update_text(gameover_sorted, "Sorted: " + std::to_string(result.sorted));
  update_text(gameover_hint, "Use Up/Down + Enter, or press R/M.");
  update_text(gameover_restart, "Restart");
  update_text(gameover_main_menu, "Main Menu");
  refresh_name_entry_lines();
  refresh_leaderboard_lines();
}

inline void set_menu_mode(MenuMode mode) {
  menu_mode = mode;
  const bool show_main = (mode == MenuMode::Main);
  const bool show_objective = (mode == MenuMode::Objective);
  const bool show_game_over = (mode == MenuMode::GameOver);

  set_line_visibility(status_line, show_main, false);
  set_line_visibility(instruction_line, show_main, false);
  set_line_visibility(difficulty_line, show_main, show_main);
  set_line_visibility(tutorial_line, show_main, show_main);
  set_line_visibility(credits_line, show_main, false);
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    set_line_visibility(level_lines[i], show_main, show_main);
  }
  set_entity_visible(character_entity, show_main);

  set_line_visibility(objective_title, show_objective, false);
  set_line_visibility(objective_level, show_objective, false);
  set_line_visibility(objective_hint, show_objective, false);
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    set_line_visibility(objective_recipe_lines[i], show_objective && i < active_objective_lines,
                        false);
  }

  set_line_visibility(gameover_title, show_game_over, false);
  set_line_visibility(gameover_level, show_game_over, false);
  set_line_visibility(gameover_collected, show_game_over, false);
  set_line_visibility(gameover_sorted, show_game_over, false);
  set_line_visibility(gameover_hint, show_game_over, false);
  const bool show_restart = show_game_over && !awaiting_name_entry;
  set_line_visibility(gameover_name_prompt, show_game_over && awaiting_name_entry, false);
  set_line_visibility(gameover_name_value, show_game_over && awaiting_name_entry, false);
  set_line_visibility(gameover_restart, show_restart, show_restart);
  set_line_visibility(gameover_main_menu, show_restart, show_restart);
  const bool show_board = show_game_over && show_leaderboard;
  set_line_visibility(leaderboard_title, show_board, false);
  for (auto& line : leaderboard_lines) {
    set_line_visibility(line, show_board, false);
  }
}

inline void start_pending_level() {
  if (!has_pending_level) return;
  if (!pending_infinite && !pending_tutorial &&
      pending_level_index >= levels::parsed_levels.size()) {
    has_pending_level = false;
    pending_infinite = false;
    pending_tutorial = false;
    enter_main_menu_mode();
    return;
  }
  lives::reset_lives();
  if (pending_tutorial) {
    levels::start_tutorial_mode();
    tutorial::start();
  } else if (pending_infinite) {
    tutorial::stop();
    levels::start_infinite_mode();
  } else {
    tutorial::stop();
    levels::start_level(pending_level_index);
  }
  player::reset_for_new_level();
  camera_shake::reset();
  vfx::reset_wobble_offsets();
  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }
  if (auto* menu_scene = scene::get_scene("menu")) {
    menu_scene->set_pause(true);
  }
  countdown::start("main", 3, []() {
    camera_shake::reset();
    vfx::reset_wobble_offsets();
    if (auto* main_scene = scene::get_scene("main")) {
      main_scene->set_pause(false);
    }
  });
  has_pending_level = false;
  pending_infinite = false;
  pending_tutorial = false;
  suppress_input_for_frames(2);
  set_menu_mode(MenuMode::Main);
}

inline void enter_objective_mode(size_t level_index) {
  if (level_index >= levels::parsed_levels.size()) return;
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  pending_level_index = level_index;
  has_pending_level = true;
  pending_infinite = false;
  pending_tutorial = false;
  refresh_objective_lines(level_index);
  set_menu_mode(MenuMode::Objective);
}

inline void enter_infinite_objective_mode() {
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  pending_level_index = levels::infinite_menu_index();
  has_pending_level = true;
  pending_infinite = true;
  pending_tutorial = false;
  levels::prepare_infinite_preview();
  const std::string label = "Infinite Mode: Round 1";
  refresh_objective_lines_from_level(levels::infinite_level, label);
  set_menu_mode(MenuMode::Objective);
}

inline void enter_tutorial_objective_mode() {
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  pending_level_index = levels::tutorial_menu_index();
  has_pending_level = true;
  pending_infinite = false;
  pending_tutorial = true;

  update_text(objective_title, "Tutorial");
  update_text(objective_level, "Learn movement, catch, strike, and familiar play");
  active_objective_lines = 6;
  update_text(objective_recipe_lines[0], "1. Move left");
  update_text(objective_recipe_lines[1], "2. Move right");
  update_text(objective_recipe_lines[2], "3. Catch a mushroom");
  update_text(objective_recipe_lines[3], "4. Shoot a mushroom");
  update_text(objective_recipe_lines[4], "5. Place familiar on the right");
  update_text(objective_recipe_lines[5], "6. Catch two together");
  for (size_t i = active_objective_lines; i < kMaxLevelLines; ++i) {
    update_text(objective_recipe_lines[i], "");
  }
  update_text(objective_hint, "Press Enter or tap to start tutorial");
  set_menu_mode(MenuMode::Objective);
}

inline void enter_game_over_mode() {
  if (!levels::last_result_valid) return;
  awaiting_name_entry = false;
  pending_leaderboard_score = 0;
  if (levels::last_result.infinite_mode) {
    pending_leaderboard_score = levels::last_result.global_score;
    if (leaderboard::qualifies(pending_leaderboard_score)) {
      awaiting_name_entry = true;
      text_input::begin(kNameMaxLength);
    } else {
      text_input::end();
    }
  } else {
    text_input::end();
  }
  refresh_gameover_lines();
  has_pending_level = false;
  pending_infinite = false;
  pending_tutorial = false;
  set_menu_mode(MenuMode::GameOver);
}

inline void enter_main_menu_mode() {
  tutorial::stop();
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  refresh_level_lines();
  refresh_instruction_line();
  refresh_difficulty_line();
  refresh_tutorial_line();
  refresh_status_line();
  has_pending_level = false;
  pending_infinite = false;
  pending_tutorial = false;
  suppress_input_for_frames(2);
  set_menu_mode(MenuMode::Main);
}

struct MenuController : public dynamic::DynamicObject {
  MenuController() : dynamic::DynamicObject() {}

  static constexpr size_t kDifficultyMainSlot = 0;
  static constexpr size_t kTutorialMainSlot = 1;
  static constexpr size_t kLevelMainSlotOffset = 2;

  void update_pointer() {
    for (const auto& evt : input::events()) {
      if (evt.kind == engine::InputKind::PointerMove ||
          evt.kind == engine::InputKind::PointerDown) {
        last_pointer = glm::vec2{static_cast<float>(evt.x), static_cast<float>(evt.y)};
      }
    }
  }

  size_t main_slot_count(size_t current_levels) const {
    return kLevelMainSlotOffset + current_levels;
  }

  std::optional<size_t> main_slot_level_index(size_t slot, size_t current_levels) const {
    if (slot < kLevelMainSlotOffset) return std::nullopt;
    const size_t level_index = slot - kLevelMainSlotOffset;
    if (level_index >= current_levels) return std::nullopt;
    return level_index;
  }

  bool is_main_slot_selectable(size_t slot, size_t current_levels) const {
    if (slot == kDifficultyMainSlot || slot == kTutorialMainSlot) {
      return true;
    }
    const auto level_index = main_slot_level_index(slot, current_levels);
    if (!level_index) return false;
    return is_selectable_level(*level_index);
  }

  std::optional<size_t> main_slot_at_point(const glm::vec2& point, size_t current_levels) const {
    if (point_hits_line(difficulty_line, point)) {
      return kDifficultyMainSlot;
    }
    if (point_hits_line(tutorial_line, point)) {
      return kTutorialMainSlot;
    }
    auto level_index = level_index_at_point(point);
    if (!level_index || *level_index >= current_levels) {
      return std::nullopt;
    }
    return *level_index + kLevelMainSlotOffset;
  }

  void ensure_main_selection(size_t current_levels) {
    const size_t slot_count = main_slot_count(current_levels);
    if (selected_main_slot && *selected_main_slot < slot_count &&
        is_main_slot_selectable(*selected_main_slot, current_levels)) {
      return;
    }
    for (size_t slot = 0; slot < slot_count; ++slot) {
      if (!is_main_slot_selectable(slot, current_levels)) continue;
      selected_main_slot = slot;
      return;
    }
    selected_main_slot.reset();
  }

  void move_main_selection(int direction, size_t current_levels) {
    if (direction == 0) return;
    const size_t slot_count = main_slot_count(current_levels);
    if (slot_count == 0) return;
    ensure_main_selection(current_levels);
    if (!selected_main_slot) return;

    const int count = static_cast<int>(slot_count);
    const int start = static_cast<int>(*selected_main_slot);
    for (int step = 1; step <= count; ++step) {
      int candidate = start + direction * step;
      while (candidate < 0) {
        candidate += count;
      }
      candidate %= count;
      if (!is_main_slot_selectable(static_cast<size_t>(candidate), current_levels)) continue;
      selected_main_slot = static_cast<size_t>(candidate);
      return;
    }
  }

  void move_gameover_selection(int direction) {
    if (direction == 0) return;
    selected_gameover_index = (selected_gameover_index == 0) ? 1 : 0;
  }

  void toggle_difficulty() {
    levels::cycle_difficulty();
    refresh_level_lines();
    refresh_instruction_line();
    refresh_difficulty_line();
    const size_t refreshed_levels = available_levels();
    ensure_main_selection(refreshed_levels);
    update_hover_state(refreshed_levels);
  }

  void handle_selected_main_action(size_t slot, size_t current_levels) {
    if (slot == kDifficultyMainSlot) {
      toggle_difficulty();
      return;
    }
    if (slot == kTutorialMainSlot) {
      enter_tutorial_objective_mode();
      return;
    }
    const auto level_index = main_slot_level_index(slot, current_levels);
    if (!level_index || !is_selectable_level(*level_index)) return;
    handle_level_selection(*level_index);
  }

  void apply_main_visuals(std::optional<size_t> hovered_slot, size_t current_levels) {
    const bool has_selection = selected_main_slot.has_value();
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      if (i >= current_levels) {
        set_line_visual_state(level_lines[i], false, false, false);
        continue;
      }
      const size_t slot = i + kLevelMainSlotOffset;
      const bool selected = selected_main_slot && slot == *selected_main_slot;
      const bool hovered = hovered_slot && slot == *hovered_slot;
      const bool dimmed = (!selected && has_selection) || !is_selectable_level(i);
      set_line_visual_state(level_lines[i], selected, hovered, dimmed);
    }
    const bool tutorial_selected = selected_main_slot && *selected_main_slot == kTutorialMainSlot;
    const bool tutorial_hovered = hovered_slot && *hovered_slot == kTutorialMainSlot;
    set_line_visual_state(tutorial_line, tutorial_selected, tutorial_hovered,
                          !tutorial_selected && has_selection);
    const bool difficulty_selected =
        selected_main_slot && *selected_main_slot == kDifficultyMainSlot;
    const bool difficulty_hovered = hovered_slot && *hovered_slot == kDifficultyMainSlot;
    set_line_visual_state(difficulty_line, difficulty_selected, difficulty_hovered,
                          !difficulty_selected && has_selection);
    set_line_visual_state(gameover_restart, false, false, false);
    set_line_visual_state(gameover_main_menu, false, false, false);
  }

  void apply_gameover_visuals(bool hover_restart, bool hover_menu) {
    const bool restart_selected = (selected_gameover_index == 0);
    const bool menu_selected = !restart_selected;
    set_line_visual_state(gameover_restart, restart_selected, hover_restart, !restart_selected);
    set_line_visual_state(gameover_main_menu, menu_selected, hover_menu, !menu_selected);
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      set_line_visual_state(level_lines[i], false, false, false);
    }
    set_line_visual_state(tutorial_line, false, false, false);
    set_line_visual_state(difficulty_line, false, false, false);
  }

  void clear_hover(size_t current_levels) {
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      set_line_hovered(objective_recipe_lines[i], false);
    }
    if (menu_mode == MenuMode::Main) {
      apply_main_visuals(std::nullopt, current_levels);
    } else {
      for (size_t i = 0; i < kMaxLevelLines; ++i) {
        set_line_visual_state(level_lines[i], false, false, false);
      }
      set_line_visual_state(tutorial_line, false, false, false);
      set_line_visual_state(difficulty_line, false, false, false);
    }
    if (menu_mode == MenuMode::GameOver && !awaiting_name_entry) {
      apply_gameover_visuals(false, false);
    } else {
      set_line_visual_state(gameover_restart, false, false, false);
      set_line_visual_state(gameover_main_menu, false, false, false);
    }
  }

  void update_hover_state(size_t current_levels) {
    if (menu_mode == MenuMode::Main) {
      ensure_main_selection(current_levels);
      std::optional<size_t> hovered_slot;
      if (last_pointer) {
        hovered_slot = main_slot_at_point(*last_pointer, current_levels);
      }
      if (hovered_slot && !is_main_slot_selectable(*hovered_slot, current_levels)) {
        hovered_slot.reset();
      }
      if (hovered_slot) {
        selected_main_slot = *hovered_slot;
      }
      apply_main_visuals(hovered_slot, current_levels);
      return;
    }

    if (menu_mode == MenuMode::GameOver) {
      if (awaiting_name_entry) {
        for (size_t i = 0; i < kMaxLevelLines; ++i) {
          set_line_visual_state(level_lines[i], false, false, false);
        }
        set_line_visual_state(gameover_restart, false, false, false);
        set_line_visual_state(gameover_main_menu, false, false, false);
        return;
      }
      bool hover_restart = false;
      bool hover_menu = false;
      if (last_pointer) {
        hover_restart = point_hits_line(gameover_restart, *last_pointer);
        hover_menu = point_hits_line(gameover_main_menu, *last_pointer);
      }
      if (hover_restart) {
        selected_gameover_index = 0;
      } else if (hover_menu) {
        selected_gameover_index = 1;
      }
      apply_gameover_visuals(hover_restart, hover_menu);
      return;
    }

    clear_hover(current_levels);
  }

  void handle_selected_gameover_action(size_t restart_index) {
    if (selected_gameover_index == 0) {
      if (levels::last_result.infinite_mode) {
        enter_infinite_objective_mode();
      } else if (levels::last_result.tutorial_mode) {
        enter_tutorial_objective_mode();
      } else {
        enter_objective_mode(restart_index);
      }
      return;
    }
    enter_main_menu_mode();
  }

  void update() override {
    const size_t current_levels = available_levels();
    if (cached_difficulty != levels::difficulty()) {
      cached_difficulty = levels::difficulty();
      refresh_level_lines();
      refresh_instruction_line();
      refresh_difficulty_line();
      ensure_main_selection(current_levels);
    }

    if (cached_level_count != current_levels) {
      cached_level_count = current_levels;
      refresh_level_lines();
      refresh_instruction_line();
      ensure_main_selection(current_levels);
    } else if (cached_last_played != levels::last_played_level_index) {
      cached_last_played = levels::last_played_level_index;
      refresh_level_lines();
    }

    if (cached_unlocked != levels::unlocked_levels()) {
      cached_unlocked = levels::unlocked_levels();
      refresh_level_lines();
      refresh_instruction_line();
      ensure_main_selection(current_levels);
    }

    if (cached_status != levels::last_game_status) {
      cached_status = levels::last_game_status;
      refresh_status_line();
    }

    if (menu_mode != previous_mode) {
      if (menu_mode == MenuMode::Main) {
        ensure_main_selection(current_levels);
      } else if (menu_mode == MenuMode::GameOver && !awaiting_name_entry) {
        selected_gameover_index = 0;
      }
      previous_mode = menu_mode;
    }

    if (levels::level_finished && levels::last_result_valid) {
      if (!completion_acknowledged) {
        if (shrooms::scenes::menu) {
          shrooms::scenes::menu->activate();
          shrooms::scenes::menu->set_pause(true);
        }
        if (shrooms::scenes::main) {
          shrooms::scenes::main->set_pause(true);
        }
        player::reset_for_new_level();
        enter_game_over_mode();
        completion_acknowledged = true;
      }
    } else {
      completion_acknowledged = false;
    }

    update_pointer();
    if (!shrooms::scenes::menu || !shrooms::scenes::menu->is_active) {
      clear_hover(current_levels);
      return;
    }
    if (block_input_frames > 0) {
      block_input_frames--;
      clear_hover(current_levels);
      return;
    }
    update_hover_state(current_levels);

    if (menu_mode == MenuMode::Main) {
      ensure_main_selection(current_levels);
      for (size_t i = 0; i < kMaxLevelLines; ++i) {
        const int key_code = (i == 9) ? '0' : static_cast<int>('1' + i);
        const bool pressed = input::get_button_state(key_code);
        if (pressed && !previous_state[i] && i < current_levels && is_selectable_level(i)) {
          previous_state[i] = pressed;
          handle_level_selection(i);
          return;
        }
        previous_state[i] = pressed;
      }

      for (const auto& evt : input::events()) {
        if (evt.kind != engine::InputKind::KeyDown) continue;
        const int key = input::normalize_key_code(evt.key_code);
        if (key == 'T') {
          enter_tutorial_objective_mode();
          return;
        }
        if (is_arrow_up_key(key)) {
          move_main_selection(-1, current_levels);
          update_hover_state(current_levels);
          continue;
        }
        if (is_arrow_down_key(key)) {
          move_main_selection(1, current_levels);
          update_hover_state(current_levels);
          continue;
        }
        if (is_confirm_key(key) && selected_main_slot) {
          handle_selected_main_action(*selected_main_slot, current_levels);
          return;
        }
      }

      for (const auto& evt : input::events()) {
        if (evt.kind != engine::InputKind::PointerDown) continue;
        const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
        auto slot = main_slot_at_point(point, current_levels);
        if (slot && is_main_slot_selectable(*slot, current_levels)) {
          selected_main_slot = *slot;
          handle_selected_main_action(*slot, current_levels);
          return;
        }
      }
      return;
    }

    if (menu_mode == MenuMode::Objective) {
      for (const auto& evt : input::events()) {
        if (evt.kind == engine::InputKind::PointerDown) {
          start_pending_level();
          return;
        }
        if (evt.kind != engine::InputKind::KeyDown) continue;
        const int key = input::normalize_key_code(evt.key_code);
        if (is_confirm_key(key)) {
          start_pending_level();
          return;
        }
      }
      return;
    }

    if (menu_mode == MenuMode::GameOver) {
      if (!levels::last_result_valid) {
        enter_main_menu_mode();
        return;
      }
      if (awaiting_name_entry) {
        text_input::update(input::events());
        refresh_name_entry_lines();
        if (text_input::consume_submitted()) {
          leaderboard::insert(text_input::value(), pending_leaderboard_score);
          awaiting_name_entry = false;
          refresh_leaderboard_lines();
          refresh_gameover_lines();
          set_menu_mode(MenuMode::GameOver);
          return;
        }
        for (const auto& evt : input::events()) {
          if (evt.kind != engine::InputKind::PointerDown) continue;
          const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
          if (point_hits_line(gameover_name_prompt, point) ||
              point_hits_line(gameover_name_value, point)) {
            text_input::begin(kNameMaxLength, text_input::value());
            break;
          }
        }
        return;
      }
      const size_t restart_index = levels::last_result.level_index;
      for (const auto& evt : input::events()) {
        if (evt.kind == engine::InputKind::PointerDown) {
          const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
          if (point_hits_line(gameover_restart, point)) {
            selected_gameover_index = 0;
            handle_selected_gameover_action(restart_index);
            return;
          }
          if (point_hits_line(gameover_main_menu, point)) {
            selected_gameover_index = 1;
            handle_selected_gameover_action(restart_index);
            return;
          }
          continue;
        }
        if (evt.kind != engine::InputKind::KeyDown) continue;
        const int key = input::normalize_key_code(evt.key_code);
        if (is_arrow_up_key(key)) {
          move_gameover_selection(-1);
          update_hover_state(current_levels);
          continue;
        }
        if (is_arrow_down_key(key)) {
          move_gameover_selection(1);
          update_hover_state(current_levels);
          continue;
        }
        if (key == 'R') {
          selected_gameover_index = 0;
          handle_selected_gameover_action(restart_index);
          return;
        }
        if (key == 'M') {
          selected_gameover_index = 1;
          handle_selected_gameover_action(restart_index);
          return;
        }
        if (key == 27) {
          selected_gameover_index = 1;
          handle_selected_gameover_action(restart_index);
          return;
        }
        if (is_confirm_key(key)) {
          handle_selected_gameover_action(restart_index);
          return;
        }
      }
    }
  }

  std::array<bool, kMaxLevelLines> previous_state{};
  size_t cached_level_count = std::numeric_limits<size_t>::max();
  size_t cached_last_played = std::numeric_limits<size_t>::max();
  size_t cached_unlocked = std::numeric_limits<size_t>::max();
  levels::Difficulty cached_difficulty = levels::Difficulty::Normal;
  std::string cached_status;
  bool completion_acknowledged = false;
  std::optional<glm::vec2> last_pointer;
  std::optional<size_t> selected_main_slot;
  size_t selected_gameover_index = 0;
  MenuMode previous_mode = MenuMode::Main;
};

inline MenuController menu_controller{};

inline TextLine make_text_line(glm::vec2 position_norm, float font_px, int layer) {
  TextLine line{};
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = shrooms::screen::norm_to_pixels(position_norm);
  auto* text_obj = arena::create<text::TextObject>("", font_px);

  entity->add(transform);
  entity->add(text_obj);
  entity->add(arena::create<layers::ConstLayer>(layer));
  line.text_hidden = arena::create<hidden::HiddenObject>();
  entity->add(line.text_hidden);
  entity->add(arena::create<scene::SceneObject>("menu"));

  line.entity = entity;
  line.transform = transform;
  line.text_object = text_obj;
  line.font_px = font_px;
  line.text_color = arena::create<LineTextColor>(line.base_text_color);
  entity->add(line.text_color);

  line.button_entity = arena::create<ecs::Entity>();
  line.button_transform = arena::create<transform::NoRotationTransform>();
  line.button_transform->pos = transform->pos;
  line.button_entity->add(line.button_transform);
  line.button_entity->add(arena::create<layers::ConstLayer>(layer - 1));
  line.button_quad = arena::create<render_system::QuadRenderable>(
      0.0f, 0.0f, engine::UIColor{0.0f, 0.0f, 0.0f, 0.0f});
  line.button_entity->add(line.button_quad);
  line.button_hidden = arena::create<hidden::HiddenObject>();
  line.button_entity->add(line.button_hidden);
  line.button_entity->add(arena::create<scene::SceneObject>("menu"));

  update_text(line, "");
  return line;
}

inline void init() {
  const glm::vec2 view_size{
      static_cast<float>(shrooms::screen::view_width),
      static_cast<float>(shrooms::screen::view_height),
  };

  menu_background = arena::create<ecs::Entity>();
  const glm::vec2 bg_size =
      shrooms::texture_sizing::from_reference_width("level_1_ezh", 312.0f);
  auto* bg_transform = arena::create<transform::NoRotationTransform>();
  bg_transform->pos =
      glm::vec2{-0.025f * view_size.x, (view_size.y - bg_size.y) * 0.5f};
  menu_background->add(bg_transform);
  menu_background->add(arena::create<layers::ConstLayer>(-2));
  const engine::TextureId bg_tex = engine::resources::register_texture("level_1_ezh");
  menu_background->add(arena::create<render_system::SpriteRenderable>(bg_tex, bg_size));
  menu_background->add(arena::create<scene::SceneObject>("menu"));
  vfx::attach_wobble(menu_background, glm::vec2{4.0f, 2.5f}, 0.18f, false);

  character_entity = arena::create<ecs::Entity>();
  const glm::vec2 character_size =
      shrooms::texture_sizing::from_reference_width("witch", 120.0f);
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos =
      shrooms::screen::center_to_top_left(shrooms::screen::norm_to_pixels(glm::vec2{0.65f, -0.2f}),
                                          character_size);
  character_entity->add(transform);
  const engine::TextureId tex_id = engine::resources::register_texture("witch");
  character_entity->add(arena::create<render_system::SpriteRenderable>(tex_id, character_size));
  character_entity->add(arena::create<layers::ConstLayer>(3));
  character_entity->add(arena::create<hidden::HiddenObject>());
  character_entity->add(arena::create<scene::SceneObject>("menu"));

  status_line = make_text_line(glm::vec2{kMenuTextX, 0.72f}, 22.0f, 6);
  instruction_line = make_text_line(glm::vec2{kMenuTextX, 0.58f}, 20.0f, 6);
  difficulty_line = make_text_line(glm::vec2{kMenuTextX, 0.43f}, 19.0f, 6);
  tutorial_line = make_text_line(glm::vec2{kMenuTextX, 0.28f}, 20.0f, 6);
  credits_line = make_text_line(glm::vec2{kMenuTextX, -0.9f}, 18.0f, 6);
  update_text(credits_line, "Game by KiK0S, art by deadmarla.");

  auto style_menu_action = [](TextLine& line) {
    line.base_button_color = engine::UIColor{0.15f, 0.15f, 0.15f, 0.9f};
    line.hover_button_color = engine::UIColor{0.28f, 0.2f, 0.35f, 0.98f};
    line.selected_button_color = line.base_button_color;
    line.dimmed_button_color = engine::UIColor{0.08f, 0.08f, 0.08f, 0.5f};
    line.selected_text_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    line.dimmed_text_color = glm::vec4{0.72f, 0.72f, 0.72f, 0.92f};
    line.selected_scale = 1.0f;
  };
  style_menu_action(difficulty_line);
  style_menu_action(tutorial_line);

  const glm::vec2 base = glm::vec2{kMenuTextX, 0.08f};
  const float spacing = 0.12f;
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    const glm::vec2 pos = base - glm::vec2(0.0f, spacing * static_cast<float>(i));
    level_lines[i] = make_text_line(pos, 20.0f, 6);
    style_menu_action(level_lines[i]);
  }

  objective_title = make_text_line(glm::vec2{kMenuTextX, 0.65f}, 24.0f, 6);
  objective_level = make_text_line(glm::vec2{kMenuTextX, 0.48f}, 20.0f, 6);
  const glm::vec2 objective_base = glm::vec2{kMenuTextX, 0.25f};
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    const glm::vec2 pos = objective_base - glm::vec2(0.0f, spacing * static_cast<float>(i));
    objective_recipe_lines[i] = make_text_line(pos, 20.0f, 6);
  }
  objective_hint = make_text_line(glm::vec2{kMenuTextX, -0.8f}, 18.0f, 6);

  gameover_title = make_text_line(glm::vec2{kMenuTextX, 0.6f}, 24.0f, 6);
  gameover_level = make_text_line(glm::vec2{kMenuTextX, 0.44f}, 20.0f, 6);
  gameover_collected = make_text_line(glm::vec2{kMenuTextX, 0.28f}, 20.0f, 6);
  gameover_sorted = make_text_line(glm::vec2{kMenuTextX, 0.14f}, 20.0f, 6);
  gameover_name_prompt = make_text_line(glm::vec2{kMenuTextX, 0.02f}, 18.0f, 6);
  gameover_name_value = make_text_line(glm::vec2{kMenuTextX, -0.1f}, 20.0f, 6);
  gameover_restart = make_text_line(glm::vec2{kMenuTextX, -0.3f}, 20.0f, 6);
  gameover_main_menu = make_text_line(glm::vec2{kMenuTextX, -0.46f}, 20.0f, 6);
  gameover_hint = make_text_line(glm::vec2{kMenuTextX, -0.75f}, 18.0f, 6);

  leaderboard_title = make_text_line(glm::vec2{kLeaderboardTextX, 0.62f}, 22.0f, 6);
  const glm::vec2 leaderboard_base = glm::vec2{kLeaderboardTextX, 0.45f};
  const float leaderboard_spacing = 0.1f;
  for (size_t i = 0; i < kLeaderboardLines; ++i) {
    const glm::vec2 pos =
        leaderboard_base - glm::vec2(0.0f, leaderboard_spacing * static_cast<float>(i));
    leaderboard_lines[i] = make_text_line(pos, 18.0f, 6);
  }

  if (gameover_restart.button_quad) {
    gameover_restart.base_button_color = engine::UIColor{0.15f, 0.15f, 0.15f, 0.95f};
    gameover_restart.hover_button_color = engine::UIColor{0.28f, 0.2f, 0.35f, 0.98f};
    gameover_restart.selected_button_color = gameover_restart.base_button_color;
    gameover_restart.dimmed_button_color = engine::UIColor{0.08f, 0.08f, 0.08f, 0.52f};
    gameover_restart.selected_text_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    gameover_restart.dimmed_text_color = glm::vec4{0.72f, 0.72f, 0.72f, 0.92f};
    gameover_restart.selected_scale = 1.0f;
    gameover_restart.button_quad->color = gameover_restart.base_button_color;
  }
  if (gameover_main_menu.button_quad) {
    gameover_main_menu.base_button_color = engine::UIColor{0.15f, 0.15f, 0.15f, 0.95f};
    gameover_main_menu.hover_button_color = engine::UIColor{0.28f, 0.2f, 0.35f, 0.98f};
    gameover_main_menu.selected_button_color = gameover_main_menu.base_button_color;
    gameover_main_menu.dimmed_button_color = engine::UIColor{0.08f, 0.08f, 0.08f, 0.52f};
    gameover_main_menu.selected_text_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    gameover_main_menu.dimmed_text_color = glm::vec4{0.72f, 0.72f, 0.72f, 0.92f};
    gameover_main_menu.selected_scale = 1.0f;
    gameover_main_menu.button_quad->color = gameover_main_menu.base_button_color;
  }

  refresh_status_line();
  refresh_instruction_line();
  refresh_level_lines();
  refresh_leaderboard_lines();
  refresh_name_entry_lines();

  enter_main_menu_mode();
}

}  // namespace menu
