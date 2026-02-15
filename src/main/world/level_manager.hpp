#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "systems/spawn/periodic_spawner_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/transformation/transform_object.hpp"
#include "utils/save_system.hpp"

#include "scoreboard.hpp"
#include "shrooms_assets.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "engine/geometry_builder.h"
#include "engine/resource_ids.h"
#include "systems/render/sprite_system.hpp"
#include "vfx.hpp"
#include "camera_shake.hpp"
#include "round_transition.hpp"
#include "leaderboard.hpp"

namespace levels {

enum class ObjectiveRule {
  CollectOnly,
  SortOnly,
};

enum class Difficulty {
  Normal,
  Easy,
};

struct SpawnerPlan {
  std::string type;
  std::string template_name;
  float period = 0.0f;
  double density = 0.0;
  int total_to_spawn = 0;
};

struct LevelDefinition {
  std::string id;
  std::vector<std::pair<std::string, int>> recipe_order;
  std::unordered_map<std::string, int> recipe;
  std::vector<SpawnerPlan> spawners;
  ObjectiveRule objective_rule = ObjectiveRule::CollectOnly;
  std::string objective_hint;
};

struct LastResult {
  bool success = false;
  int collected = 0;
  int sorted = 0;
  int global_score = 0;
  size_t level_index = 0;
  std::string level_id;
  bool infinite_mode = false;
  bool tutorial_mode = false;
  int rounds_won = 0;
  int round_index = 0;
};

enum class LossReason {
  None,
  TooMany,
  NotEnough,
  WrongAction,
  OutOfLives,
};

struct LossInfo {
  LossReason reason = LossReason::None;
  std::string type;
};

inline std::vector<LevelDefinition> parsed_levels{};
inline std::vector<LevelDefinition> base_levels{};
inline std::unordered_map<std::string, periodic_spawn::PeriodicSpawnerObject*> spawners_by_type{};
inline std::unordered_map<std::string, std::unordered_set<ecs::Entity*>> active_entities{};
inline std::unordered_map<std::string, int> collected_counts{};
inline std::unordered_map<std::string, int> sorted_counts{};
inline size_t current_level_index = 0;
inline size_t unlocked_level_count = 0;
inline size_t last_played_level_index = 0;
inline std::string last_game_status = "No games yet";
inline bool last_game_success = false;
inline bool level_failed = false;
inline bool level_finished = false;
inline LastResult last_result{};
inline bool last_result_valid = false;
inline bool game_over_pending = false;
inline LossInfo pending_loss{};
inline LossInfo last_loss{};
inline bool infinite_mode = false;
inline bool tutorial_mode = false;
inline int infinite_round_index = 0;
inline int infinite_rounds_won = 0;
inline int infinite_global_score = 0;
inline bool infinite_preview_ready = false;
inline LevelDefinition infinite_level{};
inline LevelDefinition tutorial_level{};
inline bool progress_save_exists = false;
inline std::unordered_map<std::string, SpawnerPlan> base_spawner_plans{};
inline std::vector<std::string> infinite_types{};
inline render_system::SpriteRenderable* background_sprite = nullptr;
inline transform::NoRotationTransform* background_transform = nullptr;
inline Difficulty current_difficulty = Difficulty::Normal;

using TutorialSpawnHook = std::function<void(const std::string&, ecs::Entity*)>;
using TutorialCatchHook =
    std::function<void(const std::string&, ecs::Entity*, bool from_familiar)>;
using TutorialMissHook = std::function<void(const std::string&, ecs::Entity*)>;
using TutorialSortHook = std::function<void(const std::string&, ecs::Entity*)>;
inline TutorialSpawnHook tutorial_spawn_hook{};
inline TutorialCatchHook tutorial_catch_hook{};
inline TutorialMissHook tutorial_miss_hook{};
inline TutorialSortHook tutorial_sort_hook{};

constexpr const char* kProgressKey = "shrooms_progress";
inline constexpr size_t kTutorialLevelIndexOffset = 1;

inline void set_tutorial_hooks(TutorialSpawnHook spawn_hook, TutorialCatchHook catch_hook,
                               TutorialMissHook miss_hook, TutorialSortHook sort_hook) {
  tutorial_spawn_hook = std::move(spawn_hook);
  tutorial_catch_hook = std::move(catch_hook);
  tutorial_miss_hook = std::move(miss_hook);
  tutorial_sort_hook = std::move(sort_hook);
}

inline Difficulty difficulty() { return current_difficulty; }

inline std::string difficulty_label() {
  switch (current_difficulty) {
    case Difficulty::Easy:
      return "Easy";
    case Difficulty::Normal:
    default:
      return "Normal";
  }
}

inline bool is_tutorial_mode() { return tutorial_mode; }

inline size_t tutorial_menu_index() {
  return parsed_levels.size() + kTutorialLevelIndexOffset;
}

inline const LevelDefinition* current_level() {
  if (infinite_mode) {
    return &infinite_level;
  }
  if (tutorial_mode) {
    return &tutorial_level;
  }
  if (parsed_levels.empty() || current_level_index >= parsed_levels.size()) {
    return nullptr;
  }
  return &parsed_levels[current_level_index];
}

inline size_t infinite_menu_index() {
  return parsed_levels.size();
}

inline bool infinite_available() {
  return !parsed_levels.empty();
}

inline size_t clamp_unlocked(size_t count) {
  if (parsed_levels.empty()) {
    return 0;
  }
  if (count < 1) {
    count = 1;
  }
  if (count > parsed_levels.size()) {
    count = parsed_levels.size();
  }
  return count;
}

inline bool is_unlocked(size_t index) {
  if (parsed_levels.empty()) return false;
  return index < unlocked_level_count;
}

inline size_t unlocked_levels() { return unlocked_level_count; }

inline bool has_progress_save() { return progress_save_exists; }

inline int progress_for_type(const LevelDefinition& level, const std::string& type) {
  switch (level.objective_rule) {
    case ObjectiveRule::SortOnly:
      return sorted_counts[type];
    case ObjectiveRule::CollectOnly:
    default:
      return collected_counts[type];
  }
}

inline std::string objective_line_text(const LevelDefinition& level, const std::string& type,
                                       int target) {
  if (!level.objective_hint.empty()) {
    return level.objective_hint + " " + std::to_string(target) + " " + type;
  }
  switch (level.objective_rule) {
    case ObjectiveRule::SortOnly:
      return "Shoot " + std::to_string(target) + " " + type;
    case ObjectiveRule::CollectOnly:
    default:
      return "Collect " + std::to_string(target) + " " + type;
  }
}

inline void layout_background_sprite(const std::string& texture_name) {
  if (!background_sprite || !background_transform) return;
  if (texture_name.empty()) return;

  const float view_width = static_cast<float>(shrooms::screen::view_width);
  const float view_height = static_cast<float>(shrooms::screen::view_height);
  const glm::vec2 size = shrooms::texture_sizing::from_width_px(texture_name, view_width);
  // SpriteRenderable keeps its own quad geometry; resizing requires rebuilding it.
  background_sprite->geometry = engine::geometry::make_quad(size.x, size.y);
  background_sprite->uploaded = false;
  background_sprite->size = size;
  background_transform->pos = glm::vec2{0.0f, view_height - size.y};
}

inline void register_background_sprite(render_system::SpriteRenderable* sprite,
                                       transform::NoRotationTransform* transform,
                                       const std::string& texture_name) {
  background_sprite = sprite;
  background_transform = transform;
  layout_background_sprite(texture_name);
}

inline void set_background_texture(const std::string& texture_name) {
  if (!background_sprite) return;
  if (texture_name.empty()) return;
  const engine::TextureId tex_id = engine::resources::register_texture(texture_name);
  if (tex_id != engine::kInvalidTextureId) {
    background_sprite->texture_id = tex_id;
    layout_background_sprite(texture_name);
  }
}

inline void apply_level_background(const LevelDefinition& level) {
  if (infinite_mode) {
    if (!parsed_levels.empty()) {
      set_background_texture(parsed_levels.back().id);
    }
    return;
  }
  if (tutorial_mode) {
    if (!parsed_levels.empty()) {
      set_background_texture(parsed_levels.front().id);
    } else {
      set_background_texture("level_1_ezh");
    }
    return;
  }
  set_background_texture(level.id);
}

inline bool has_pending_failure() { return game_over_pending; }

inline const LossInfo& pending_loss_info() { return pending_loss; }

inline const LossInfo& last_loss_info() { return last_loss; }

inline std::string loss_reason_label(const LossInfo& info) {
  switch (info.reason) {
    case LossReason::TooMany: {
      if (!info.type.empty()) {
        return "Too many " + info.type;
      }
      return "Too many mushrooms";
    }
    case LossReason::NotEnough: {
      if (!info.type.empty()) {
        return "Not enough " + info.type;
      }
      return "Not enough mushrooms";
    }
    case LossReason::WrongAction: {
      if (!info.type.empty()) {
        return "Wrong action for " + info.type;
      }
      return "Wrong action";
    }
    case LossReason::OutOfLives:
      return "Out of lives";
    case LossReason::None:
    default:
      return "Game over";
  }
}

inline void save_progress() {
  if (parsed_levels.empty()) return;
  save::write_text(kProgressKey, std::to_string(unlocked_level_count));
}

inline void load_progress() {
  unlocked_level_count = parsed_levels.empty() ? 0 : 1;
  auto saved = save::read_text(kProgressKey);
  progress_save_exists = saved.has_value();
  if (!saved) return;
  std::istringstream in(*saved);
  size_t count = 0;
  if (!(in >> count)) {
    return;
  }
  unlocked_level_count = clamp_unlocked(count);
}

inline void unlock_next_level(size_t level_index) {
  if (parsed_levels.empty()) return;
  const size_t target = level_index + 2;
  const size_t clamped = clamp_unlocked(target);
  if (clamped > unlocked_level_count) {
    unlocked_level_count = clamped;
    save_progress();
  }
}

inline void reset_active_entities() {
  for (auto& [type, entities] : active_entities) {
    for (auto* entity : entities) {
      if (entity) {
        entity->mark_deleted();
      }
    }
  }
  active_entities.clear();
}

inline uint32_t fnv1a_append(uint32_t hash, const std::string& text) {
  for (unsigned char c : text) {
    hash ^= static_cast<uint32_t>(c);
    hash *= 16777619u;
  }
  return hash;
}

inline uint32_t seed_for_level(const LevelDefinition& level, size_t seed_index,
                               const std::string& spawner_type) {
  uint32_t hash = 2166136261u;
  hash = fnv1a_append(hash, level.id);
  hash = fnv1a_append(hash, spawner_type);
  hash ^= static_cast<uint32_t>(seed_index + 1) * 0x9e3779b9u;
  return hash;
}

inline void seed_spawners_for_level(const LevelDefinition& level, size_t seed_index) {
  for (const auto& plan : level.spawners) {
    auto it = spawners_by_type.find(plan.type);
    if (it == spawners_by_type.end()) continue;
    auto* spawner = it->second;
    if (!spawner) continue;
    const uint32_t seed = seed_for_level(level, seed_index, plan.type);
    spawner->reseed(seed);
  }
}

inline void build_infinite_spawner_cache() {
  base_spawner_plans.clear();
  infinite_types.clear();
  for (const auto& level : parsed_levels) {
    for (const auto& plan : level.spawners) {
      if (base_spawner_plans.find(plan.type) != base_spawner_plans.end()) continue;
      base_spawner_plans[plan.type] = plan;
      infinite_types.push_back(plan.type);
    }
  }
}

inline SpawnerPlan make_easy_spawner(SpawnerPlan plan, int target) {
  plan.period = std::max(1.55f, plan.period * 1.12f);
  plan.density = std::max(0.16, std::min(0.62, plan.density * 0.58));
  const int spare = std::max(1, target / 3);
  plan.total_to_spawn = target + spare;
  return plan;
}

inline void reset_level_defaults(LevelDefinition& level) {
  level.objective_rule = ObjectiveRule::CollectOnly;
  level.objective_hint.clear();
}

inline ObjectiveRule easy_goal_rule_for_index(size_t index) {
  return (index % 2 == 0) ? ObjectiveRule::CollectOnly : ObjectiveRule::SortOnly;
}

inline void apply_easy_profile(LevelDefinition& level, size_t level_index) {
  if (level.spawners.empty()) return;
  const size_t focus_index = level_index % level.spawners.size();
  const auto& focus_plan = level.spawners[focus_index];
  const std::string focus_type = focus_plan.type;

  int base_target = 3;
  auto recipe_it = level.recipe.find(focus_type);
  if (recipe_it != level.recipe.end()) {
    base_target = recipe_it->second;
  } else if (!level.recipe_order.empty()) {
    base_target = level.recipe_order.front().second;
  }
  const int target = std::max(2, std::min(5, (base_target / 2) + 1));

  level.recipe.clear();
  level.recipe_order.clear();
  level.recipe[focus_type] = target;
  level.recipe_order.emplace_back(focus_type, target);
  level.spawners = {make_easy_spawner(focus_plan, target)};
  level.objective_rule = easy_goal_rule_for_index(level_index);
  level.objective_hint =
      (level.objective_rule == ObjectiveRule::CollectOnly) ? "Collect" : "Shoot";
}

inline void apply_difficulty_to_levels() {
  parsed_levels = base_levels;
  for (auto& level : parsed_levels) {
    reset_level_defaults(level);
  }
  if (current_difficulty == Difficulty::Easy) {
    for (size_t i = 0; i < parsed_levels.size(); ++i) {
      apply_easy_profile(parsed_levels[i], i);
    }
  }
  unlocked_level_count = clamp_unlocked(unlocked_level_count);
  build_infinite_spawner_cache();
}

inline void set_difficulty(Difficulty new_difficulty) {
  if (current_difficulty == new_difficulty) return;
  current_difficulty = new_difficulty;
  infinite_preview_ready = false;
  apply_difficulty_to_levels();
}

inline void cycle_difficulty() {
  set_difficulty(current_difficulty == Difficulty::Normal ? Difficulty::Easy
                                                          : Difficulty::Normal);
}

inline int infinite_target_for_round(int round_index) {
  const int round_boost = std::min(3, round_index / 3);
  const int base = 2 + round_boost;
  const int jitter = rnd::get_int(0, 2);
  return base + jitter;
}

inline void build_infinite_level(int round_index) {
  infinite_level = LevelDefinition{};
  infinite_level.id = "Infinite";
  infinite_level.recipe.clear();
  infinite_level.recipe_order.clear();
  infinite_level.spawners.clear();

  if (infinite_types.empty()) {
    build_infinite_spawner_cache();
  }
  if (infinite_types.empty()) {
    return;
  }

  if (current_difficulty == Difficulty::Easy) {
    const std::string& type = infinite_types[static_cast<size_t>(round_index) % infinite_types.size()];
    const int target = std::max(2, std::min(6, 2 + round_index / 3));
    infinite_level.recipe[type] = target;
    infinite_level.recipe_order.emplace_back(type, target);
    infinite_level.objective_rule = easy_goal_rule_for_index(static_cast<size_t>(round_index));
    infinite_level.objective_hint =
        (infinite_level.objective_rule == ObjectiveRule::CollectOnly) ? "Collect" : "Shoot";

    SpawnerPlan plan{};
    auto base_it = base_spawner_plans.find(type);
    if (base_it != base_spawner_plans.end()) {
      plan = base_it->second;
    } else {
      plan.type = type;
      plan.template_name = type + "_spawned";
      plan.period = 2.0f;
      plan.density = 0.5;
      plan.total_to_spawn = target + 3;
    }
    infinite_level.spawners.push_back(make_easy_spawner(plan, target));
    return;
  }

  for (const auto& type : infinite_types) {
    const int target = infinite_target_for_round(round_index);
    infinite_level.recipe[type] = target;
    infinite_level.recipe_order.emplace_back(type, target);

    SpawnerPlan plan{};
    plan.type = type;
    auto base_it = base_spawner_plans.find(type);
    if (base_it != base_spawner_plans.end()) {
      plan.template_name = base_it->second.template_name;
      plan.period = base_it->second.period;
      plan.density = base_it->second.density;
    } else {
      plan.template_name = type + "_spawned";
      plan.period = 2.0f;
      plan.density = 0.7;
    }
    const int spare = std::max(2, target / 2);
    plan.total_to_spawn = target + spare;
    infinite_level.spawners.push_back(plan);
  }
  infinite_level.objective_rule = ObjectiveRule::CollectOnly;
  infinite_level.objective_hint.clear();
}

inline void prepare_infinite_preview() {
  infinite_round_index = 0;
  infinite_rounds_won = 0;
  build_infinite_level(infinite_round_index);
  infinite_preview_ready = true;
}

inline void parse_levels(const std::string& filename) {
  parsed_levels.clear();
  base_levels.clear();
  std::ifstream in(filename);
  if (!in.is_open()) {
    std::cerr << "Failed to open level config: " << filename << std::endl;
    return;
  }

  LevelDefinition current{};
  std::string token;
  while (in >> token) {
    if (token == "level") {
      if (!current.id.empty()) {
        parsed_levels.push_back(current);
      }
      current = LevelDefinition{};
      in >> current.id;
    } else if (token == "recipe") {
      std::string type;
      int count = 0;
      in >> type >> count;
      current.recipe[type] = count;
      current.recipe_order.emplace_back(type, count);
    } else if (token == "spawn") {
      SpawnerPlan plan{};
      in >> plan.type >> plan.template_name >> plan.period >> plan.density >> plan.total_to_spawn;
      current.spawners.push_back(plan);
    } else if (token == "level_end") {
      if (!current.id.empty()) {
        parsed_levels.push_back(current);
      }
      current = LevelDefinition{};
    }
  }
  if (!current.id.empty()) {
    parsed_levels.push_back(current);
  }
  base_levels = parsed_levels;
  apply_difficulty_to_levels();
}

inline void register_spawner(periodic_spawn::PeriodicSpawnerObject* spawner) {
  if (!spawner) return;
  if (spawner->spawn_type.empty()) {
    std::cerr << "Spawner registered without type, ignoring" << std::endl;
    return;
  }
  spawners_by_type[spawner->spawn_type] = spawner;
}

inline void update_scoreboard_for(const std::string& type) {
  auto* level = current_level();
  if (!level) return;
  auto target_it = level->recipe.find(type);
  if (target_it == level->recipe.end()) return;
  scoreboard::update_score(type, progress_for_type(*level, type), target_it->second);
}

inline void check_completion();
inline void trigger_failure(LossReason reason, const std::string& type = "");
inline void finalize_level(bool success);

inline ecs::Entity* spawn_mushroom_now(const std::string& type, const glm::vec2& center_px) {
  auto it = spawners_by_type.find(type);
  if (it == spawners_by_type.end() || !it->second) return nullptr;
  auto* spawner = it->second;
  return spawner->rule.spawn(center_px);
}

inline std::string default_spawn_type() {
  for (const auto& level : parsed_levels) {
    if (!level.spawners.empty()) {
      return level.spawners.front().type;
    }
  }
  if (!spawners_by_type.empty()) {
    return spawners_by_type.begin()->first;
  }
  return "mukhomor";
}

inline void on_mushroom_spawned(const std::string& type, ecs::Entity* entity) {
  if (tutorial_spawn_hook) {
    tutorial_spawn_hook(type, entity);
  }
  if (!current_level()) return;
  active_entities[type].insert(entity);
}

inline void on_mushroom_caught(
    const std::string& type, ecs::Entity* entity,
    float player_center_x = std::numeric_limits<float>::quiet_NaN(), bool from_familiar = false) {
  if (!entity || entity->is_pending_deletion()) return;
  if (vfx::is_catch_animating(entity)) return;
  if (tutorial_catch_hook) {
    tutorial_catch_hook(type, entity, from_familiar);
  }
  auto* level = current_level();
  vfx::spawn_catch_effect(entity, player_center_x);
  if (!level) return;

  auto active_it = active_entities.find(type);
  if (active_it != active_entities.end()) {
    active_it->second.erase(entity);
  }
  camera_shake::add_trauma(0.12f);

  auto recipe_it = level->recipe.find(type);
  if (recipe_it == level->recipe.end()) {
    check_completion();
    return;
  }
  if (infinite_mode) {
    infinite_global_score += 10;
  }
  collected_counts[type] += 1;
  update_scoreboard_for(type);
  if (progress_for_type(*level, type) > recipe_it->second) {
    trigger_failure(LossReason::TooMany, type);
    return;
  }
  check_completion();
}

inline void on_mushroom_missed(const std::string& type, ecs::Entity* entity) {
  if (!entity || entity->is_pending_deletion()) return;
  if (vfx::is_catch_animating(entity)) return;
  if (tutorial_miss_hook) {
    tutorial_miss_hook(type, entity);
  }
  if (!current_level()) return;
  auto active_it = active_entities.find(type);
  if (active_it != active_entities.end()) {
    active_it->second.erase(entity);
  }
  vfx::spawn_miss_effect(entity);
  camera_shake::add_trauma(0.06f);
  check_completion();
}

inline void on_mushroom_sorted(ecs::Entity* entity) {
  if (!entity) return;
  auto* level = current_level();
  if (!level) {
    entity->mark_deleted();
    return;
  }
  auto* sprite = entity->get<render_system::SpriteRenderable>();
  std::string type = sprite ? engine::resources::texture_name(sprite->texture_id) : "";
  if (tutorial_sort_hook) {
    tutorial_sort_hook(type, entity);
  }
  auto active_it = active_entities.find(type);
  if (active_it != active_entities.end()) {
    active_it->second.erase(entity);
  }
  auto recipe_it = level->recipe.find(type);
  if (infinite_mode) {
    infinite_global_score += 7;
  }
  sorted_counts[type] += 1;
  update_scoreboard_for(type);
  if (recipe_it != level->recipe.end() && progress_for_type(*level, type) > recipe_it->second) {
    trigger_failure(LossReason::TooMany, type);
  }
  vfx::spawn_destroy_effect(entity);
  camera_shake::add_trauma(0.1f);
  entity->mark_deleted();
  check_completion();
}

inline void configure_spawners_for_level(const LevelDefinition& level) {
  for (auto& [type, spawner] : spawners_by_type) {
    if (!spawner) continue;
    spawner->enabled = false;
  }
  for (const auto& plan : level.spawners) {
    auto it = spawners_by_type.find(plan.type);
    if (it == spawners_by_type.end()) {
      std::cerr << "No spawner registered for type " << plan.type << std::endl;
      continue;
    }
    auto* spawner = it->second;
    spawner->configure(plan.period, plan.density, plan.total_to_spawn);
    spawner->enabled = true;
  }
}

inline void start_level_with_definition(const LevelDefinition& level, size_t display_index,
                                        size_t seed_index, const std::string& status_label) {
  current_level_index = display_index;
  level_failed = false;
  level_finished = false;
  game_over_pending = false;
  pending_loss = LossInfo{};
  reset_active_entities();

  last_played_level_index = display_index;
  last_game_status = status_label;
  last_game_success = false;
  collected_counts.clear();
  sorted_counts.clear();
  for (const auto& [type, target] : level.recipe_order) {
    collected_counts[type] = 0;
    sorted_counts[type] = 0;
  }
  scoreboard::init_with_targets(level.recipe_order);
  configure_spawners_for_level(level);
  seed_spawners_for_level(level, seed_index);
  for (const auto& [type, target] : level.recipe_order) {
    update_scoreboard_for(type);
  }
  apply_level_background(level);

  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(false);
  }
}

inline void start_infinite_mode() {
  tutorial_mode = false;
  if (!infinite_preview_ready) {
    infinite_round_index = 0;
    infinite_rounds_won = 0;
    build_infinite_level(infinite_round_index);
  }
  infinite_mode = true;
  infinite_preview_ready = false;
  infinite_global_score = 0;
  const std::string status = "Infinite run: round " + std::to_string(infinite_round_index + 1) +
                             " (score " + std::to_string(infinite_global_score) + ")";
  start_level_with_definition(infinite_level, infinite_menu_index(),
                              static_cast<size_t>(infinite_round_index), status);
}

inline void advance_infinite_round() {
  infinite_global_score += 100;
  infinite_rounds_won += 1;
  infinite_round_index += 1;
  build_infinite_level(infinite_round_index);
  const std::string status = "Infinite run: round " + std::to_string(infinite_round_index + 1) +
                             " (score " + std::to_string(infinite_global_score) + ")";
  start_level_with_definition(infinite_level, infinite_menu_index(),
                              static_cast<size_t>(infinite_round_index), status);
}

inline void start_level(size_t index) {
  infinite_mode = false;
  tutorial_mode = false;
  infinite_preview_ready = false;
  if (parsed_levels.empty()) {
    std::cerr << "No levels parsed" << std::endl;
    return;
  }
  if (index >= parsed_levels.size()) {
    index = 0;
  }
  const auto& level = parsed_levels[index];
  const std::string status = "Playing " + level.id;
  start_level_with_definition(level, index, index, status);
}

inline void start_tutorial_mode() {
  infinite_mode = false;
  infinite_preview_ready = false;
  tutorial_mode = true;
  tutorial_level = LevelDefinition{};
  if (!parsed_levels.empty()) {
    tutorial_level.id = parsed_levels.front().id;
  } else {
    tutorial_level.id = "level_1_ezh";
  }
  tutorial_level.objective_rule = ObjectiveRule::CollectOnly;
  tutorial_level.objective_hint.clear();
  const std::string status = "Playing tutorial";
  start_level_with_definition(tutorial_level, tutorial_menu_index(), 0, status);
}

inline void finish_tutorial(bool success) {
  if (!tutorial_mode) return;
  if (level_finished) return;
  level_failed = !success;
  level_finished = true;
  finalize_level(success);
}

inline void restart_level() {
  if (infinite_mode) {
    infinite_round_index = 0;
    infinite_rounds_won = 0;
    infinite_preview_ready = false;
    start_infinite_mode();
    return;
  }
  if (tutorial_mode) {
    start_tutorial_mode();
    return;
  }
  start_level(current_level_index);
}

inline void advance_level() {
  if (infinite_mode) {
    advance_infinite_round();
    return;
  }
  if (tutorial_mode) {
    start_tutorial_mode();
    return;
  }
  size_t next_index = current_level_index + 1;
  if (next_index >= parsed_levels.size()) {
    next_index = 0;
  }
  start_level(next_index);
}

inline bool all_spawns_exhausted() {
  auto* level = current_level();
  if (!level) return false;
  for (const auto& plan : level->spawners) {
    auto it = spawners_by_type.find(plan.type);
    if (it == spawners_by_type.end()) continue;
    auto* spawner = it->second;
    if (!spawner) continue;
    if (!spawner->is_depleted()) {
      return false;
    }
    if (!active_entities[plan.type].empty()) {
      return false;
    }
  }
  return true;
}

inline int remaining_spawns_for(const std::string& type) {
  auto it = spawners_by_type.find(type);
  if (it == spawners_by_type.end()) return 0;
  auto* spawner = it->second;
  if (!spawner) return 0;
  if (spawner->max_spawn_count < 0) {
    return -1;
  }
  return std::max(0, spawner->max_spawn_count - spawner->spawned_count);
}

inline LossInfo evaluate_loss_info() {
  LossInfo info{};
  if (tutorial_mode) return info;
  auto* level = current_level();
  if (!level) return info;
  for (const auto& [type, target] : level->recipe_order) {
    const int progress = progress_for_type(*level, type);
    if (progress > target) {
      info.reason = LossReason::TooMany;
      info.type = type;
      return info;
    }
    int active_count = 0;
    auto active_it = active_entities.find(type);
    if (active_it != active_entities.end()) {
      active_count = static_cast<int>(active_it->second.size());
    }
    const int remaining = remaining_spawns_for(type);
    if (remaining < 0) {
      continue;
    }
    if (progress + active_count + remaining < target) {
      info.reason = LossReason::NotEnough;
      info.type = type;
      return info;
    }
  }
  return info;
}

inline void trigger_failure(LossReason reason, const std::string& type) {
  if (tutorial_mode) return;
  if (level_finished || game_over_pending) return;
  level_failed = true;
  game_over_pending = true;
  pending_loss.reason = reason;
  pending_loss.type = type;
  for (auto& [_, spawner] : spawners_by_type) {
    if (spawner) {
      spawner->enabled = false;
    }
  }
}

inline void finalize_level(bool success) {
  auto* level = current_level();
  if (!level) return;

  int total_sorted = 0;
  for (const auto& [type, sorted] : sorted_counts) {
    total_sorted += sorted;
  }

  int total_collected = 0;
  for (const auto& [type, collected] : collected_counts) {
    total_collected += collected;
  }

  last_result.success = success;
  last_result.collected = total_collected;
  last_result.sorted = total_sorted;
  last_result.global_score = infinite_mode ? infinite_global_score : 0;
  last_result.level_index = current_level_index;
  last_result.level_id = level->id;
  last_result.infinite_mode = infinite_mode;
  last_result.tutorial_mode = tutorial_mode;
  last_result.rounds_won = infinite_rounds_won;
  last_result.round_index = infinite_round_index + 1;
  last_result_valid = true;
  if (success) {
    last_loss = LossInfo{};
  }

  if (infinite_mode) {
    last_result.level_index = infinite_menu_index();
    last_result.level_id = "Infinite Mode";
    last_game_status = "Infinite run ended (score " + std::to_string(infinite_global_score) + ")";
    last_game_success = success;
  } else if (tutorial_mode) {
    last_result.level_index = tutorial_menu_index();
    last_result.level_id = "Tutorial";
    last_game_status = success ? "Completed Tutorial" : "Failed Tutorial";
    last_game_success = success;
  } else if (success) {
    last_game_status = "Completed " + level->id + " (collected " +
                       std::to_string(total_collected) + ", sorted " +
                       std::to_string(total_sorted) + ")";
    last_game_success = true;
    unlock_next_level(current_level_index);
  } else {
    last_game_status = "Failed " + level->id + " (collected " +
                       std::to_string(total_collected) + ", sorted " +
                       std::to_string(total_sorted) + ")";
    last_game_success = false;
  }

  for (auto& [type, spawner] : spawners_by_type) {
    if (spawner) {
      spawner->enabled = false;
    }
  }
  reset_active_entities();
  if (infinite_mode) {
    infinite_mode = false;
  }
  if (tutorial_mode) {
    tutorial_mode = false;
  }
}

inline void finalize_failure() {
  if (!game_over_pending) return;
  last_loss = pending_loss;
  pending_loss = LossInfo{};
  game_over_pending = false;
  level_failed = true;
  level_finished = true;
  finalize_level(false);
}

inline void fail_level(LossReason reason = LossReason::None, const std::string& type = "") {
  if (tutorial_mode) return;
  trigger_failure(reason, type);
}

inline void check_completion() {
  if (tutorial_mode) return;
  if (level_finished || game_over_pending) return;
  if (!all_spawns_exhausted()) return;

  auto* level = current_level();
  if (!level) return;

  bool success = !level_failed;
  for (const auto& [type, target] : level->recipe_order) {
    if (progress_for_type(*level, type) < target) {
      success = false;
      break;
    }
  }
  if (infinite_mode) {
    if (success) {
      if (!round_transition::is_active()) {
        const int current_round = infinite_round_index + 1;
        const int next_round = current_round + 1;
        round_transition::start_round_win(current_round, next_round, []() {
          advance_infinite_round();
        });
      }
      return;
    }
    LossInfo info = evaluate_loss_info();
    if (info.reason == LossReason::None) {
      info.reason = LossReason::NotEnough;
    }
    trigger_failure(info.reason, info.type);
    return;
  }
  if (success) {
    level_failed = false;
    level_finished = true;
    finalize_level(true);
    return;
  }
  LossInfo info = evaluate_loss_info();
  if (info.reason == LossReason::None) {
    info.reason = LossReason::NotEnough;
  }
  trigger_failure(info.reason, info.type);
}

inline void initialize() {
  current_difficulty = Difficulty::Normal;
  parse_levels(shrooms::asset_path("levels.data"));
  build_infinite_spawner_cache();
  leaderboard::load_or_default();
  level_finished = true;
  level_failed = false;
  game_over_pending = false;
  pending_loss = LossInfo{};
  last_loss = LossInfo{};
  current_level_index = 0;
  last_result_valid = false;
  last_result = LastResult{};
  infinite_mode = false;
  tutorial_mode = false;
  infinite_round_index = 0;
  infinite_rounds_won = 0;
  infinite_global_score = 0;
  infinite_preview_ready = false;
  infinite_level = LevelDefinition{};
  tutorial_level = LevelDefinition{};
  progress_save_exists = false;
  load_progress();
  active_entities.clear();
  collected_counts.clear();
  sorted_counts.clear();
  if (parsed_levels.empty()) {
    last_game_status = "No levels configured";
  } else {
    last_game_status = "Select a level to begin";
  }
  last_game_success = false;
}

}  // namespace levels
