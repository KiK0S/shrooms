#pragma once

#include <cmath>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include "../definitions/components/dynamic_object.hpp"
#include "../definitions/components/init_object.hpp"
#include "../definitions/components/layered_object.hpp"
#include "../definitions/components/periodic_spawner_object.hpp"
#include "../definitions/components/scene_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include "../definitions/components/transform_object.hpp"
#include "../definitions/systems/level_loader_system.hpp"
#include "../definitions/components/textured_object.hpp"
#include "../declarations/color_system.hpp"
#include "../declarations/shader_system.hpp"
#include "../geometry/quad.hpp"
#include "../utils/file_system.hpp"
#include "../utils/random.hpp"
#include "scoreboard.hpp"

namespace {

using Clock = std::chrono::steady_clock;

float ease_out(float t) {
    return t * t * (3.0f - 2.0f * t);
}

glm::vec2 random_direction() {
    glm::vec2 dir(static_cast<float>(rnd::get_double(-1.0, 1.0)),
                  static_cast<float>(rnd::get_double(-1.0, 1.0)));
    if (glm::length(dir) < 0.001f) {
        dir = glm::vec2(1.0f, 0.0f);
    }
    return glm::normalize(dir);
}

struct SpawnAnimation : public dynamic::DynamicObject {
    explicit SpawnAnimation(transform::NoRotationTransform* transform)
        : dynamic::DynamicObject(5), transform(transform) {}

    void update() override {
        if (!transform || finished) {
            return;
        }
        if (!initialized) {
            start_time = Clock::now();
            target_scale = transform->scale_;
            transform->scale_ = target_scale * 0.05f;
            jitter_direction = random_direction();
            initialized = true;
        }

        auto now = Clock::now();
        float elapsed = std::chrono::duration<float>(now - start_time).count();
        float t = std::min(elapsed / duration, 1.0f);
        float eased = ease_out(t);

        transform->scale_ = target_scale * eased;

        glm::vec2 desired_jitter = jitter_direction * std::sin(elapsed * jitter_frequency) * (1.0f - eased) * jitter_amplitude;
        transform->pos += desired_jitter - last_jitter;
        last_jitter = desired_jitter;

        if (t >= 1.0f) {
            transform->scale_ = target_scale;
            transform->pos -= last_jitter;
            last_jitter = glm::vec2(0.0f);
            finished = true;
        }
    }

    transform::NoRotationTransform* transform = nullptr;
    glm::vec2 target_scale{1.0f};
    glm::vec2 last_jitter{0.0f};
    glm::vec2 jitter_direction{1.0f, 0.0f};
    Clock::time_point start_time{};
    bool initialized = false;
    bool finished = false;
    const float duration = 0.35f;
    const float jitter_amplitude = 0.08f;
    const float jitter_frequency = 18.0f;
};

struct ExplosionEffect : public dynamic::DynamicObject {
    ExplosionEffect(transform::NoRotationTransform* transform, glm::vec2 peak_scale, glm::vec2 jitter_base)
        : dynamic::DynamicObject(15), transform(transform), peak_scale(peak_scale), jitter_base(jitter_base) {}

    void update() override {
        if (!transform) {
            return;
        }
        auto* entity = transform->get_entity();
        if (!entity) {
            transform = nullptr;
            return;
        }

        auto now = Clock::now();
        if (!initialized) {
            start_time = now;
            transform->scale_ = peak_scale * 0.2f;
            initialized = true;
        }

        float elapsed = std::chrono::duration<float>(now - start_time).count();
        if (elapsed >= total_duration) {
            entity->mark_deleted();
            transform = nullptr;
            return;
        }

        float scale_factor = 0.0f;
        if (elapsed <= growth_duration) {
            float t = std::min(elapsed / growth_duration, 1.0f);
            scale_factor = ease_out(t);
        } else {
            float t = std::min((elapsed - growth_duration) / (total_duration - growth_duration), 1.0f);
            scale_factor = 1.0f - (t * t);
        }
        scale_factor = std::max(scale_factor, 0.0f);
        transform->scale_ = peak_scale * scale_factor;

        glm::vec2 desired_jitter = jitter_base * std::sin(elapsed * jitter_frequency);
        transform->pos += desired_jitter - last_jitter;
        last_jitter = desired_jitter;
    }

    transform::NoRotationTransform* transform = nullptr;
    glm::vec2 peak_scale{1.0f};
    glm::vec2 jitter_base{0.0f};
    glm::vec2 last_jitter{0.0f};
    bool initialized = false;
    Clock::time_point start_time{};
    const float growth_duration = 0.12f;
    const float total_duration = 0.4f;
    const float jitter_frequency = 22.0f;
};

transform::NoRotationTransform* extract_transform(ecs::Entity* entity) {
    if (!entity) {
        return nullptr;
    }
    if (auto* transform = entity->get<transform::LocalRotationTransform>()) {
        return transform;
    }
    return entity->get<transform::NoRotationTransform>();
}

void attach_spawn_animation(ecs::Entity* entity) {
    if (auto* transform = extract_transform(entity)) {
        auto animation = arena::create<SpawnAnimation>(transform);
        entity->add(animation);
    }
}

void spawn_explosion_effect(const glm::vec2& position, const glm::vec2& base_scale, bool red_tint) {
    auto explosion_entity = arena::create<ecs::Entity>();
    explosion_entity->add(&geometry::quad);
    explosion_entity->add(arena::create<layers::ConstLayer>(6));
    explosion_entity->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));

    auto transform = arena::create<transform::NoRotationTransform>();
    transform->translate(position);
    transform->scale(glm::vec2(0.001f));
    explosion_entity->add(transform);

    explosion_entity->add(arena::create<shaders::ModelMatrix>());
    explosion_entity->add(arena::create<texture::OneTextureObject>("explosion"));
    if (red_tint) {
        explosion_entity->add(arena::create<color::OneColor>(glm::vec4(1.0f, 0.25f, 0.25f, 1.0f)));
    } else {
        explosion_entity->add(&color::white);
    }
    explosion_entity->add(arena::create<scene::SceneObject>("main"));

    auto effect = arena::create<ExplosionEffect>(transform, base_scale * 1.4f, random_direction() * 0.05f);
    explosion_entity->add(effect);
    explosion_entity->bind();
}

} // namespace

namespace levels {

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
};

std::vector<LevelDefinition> parsed_levels;
std::unordered_map<std::string, periodic_spawn::PeriodicSpawnerObject*> spawners_by_type;
std::unordered_map<std::string, std::unordered_set<ecs::Entity*>> active_entities;
std::unordered_map<std::string, int> collected_counts;
std::unordered_map<std::string, int> sorted_counts;
size_t current_level_index = 0;
size_t last_played_level_index = 0;
std::string last_game_status = "No games yet";
bool last_game_success = false;
bool level_failed = false;
bool level_finished = false;

const LevelDefinition* current_level() {
    if (parsed_levels.empty() || current_level_index >= parsed_levels.size()) {
        return nullptr;
    }
    return &parsed_levels[current_level_index];
}

void reset_active_entities() {
    for (auto& [type, entities] : active_entities) {
        for (auto* entity : entities) {
            if (entity) {
                entity->mark_deleted();
            }
        }
    }
    active_entities.clear();
}

void parse_levels(const std::string& filename) {
    parsed_levels.clear();
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Failed to open level config: " << filename << std::endl;
        return;
    }

    LevelDefinition current;
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
            SpawnerPlan plan;
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
}

void register_spawner(periodic_spawn::PeriodicSpawnerObject* spawner) {
    if (!spawner) return;
    if (spawner->spawn_type.empty()) {
        std::cerr << "Spawner registered without type, ignoring" << std::endl;
        return;
    }
    spawners_by_type[spawner->spawn_type] = spawner;
}

void update_scoreboard_for(const std::string& type) {
    auto* level = current_level();
    if (!level) return;
    auto target_it = level->recipe.find(type);
    if (target_it == level->recipe.end()) return;
    scoreboard::update_score(type, collected_counts[type], target_it->second);
}

void check_completion();

void on_mushroom_spawned(const std::string& type, ecs::Entity* entity) {
    if (!current_level()) return;
    active_entities[type].insert(entity);
    attach_spawn_animation(entity);
}

void on_mushroom_caught(const std::string& type, ecs::Entity* entity) {
    auto* level = current_level();
    if (!level) return;
    auto active_it = active_entities.find(type);
    if (active_it != active_entities.end()) {
        active_it->second.erase(entity);
    }

    auto recipe_it = level->recipe.find(type);
    if (recipe_it == level->recipe.end()) {
        check_completion();
        return;
    }
    collected_counts[type] += 1;
    update_scoreboard_for(type);
    if (collected_counts[type] > recipe_it->second) {
        level_failed = true;
    }
    check_completion();
}

void on_mushroom_missed(const std::string& type, ecs::Entity* entity) {
    if (!current_level()) return;
    auto active_it = active_entities.find(type);
    if (active_it != active_entities.end()) {
        active_it->second.erase(entity);
    }
    if (auto* transform = extract_transform(entity)) {
        glm::vec2 pos = transform->pos;
        glm::vec2 scale(std::max(transform->scale_.x, 0.06f), std::max(transform->scale_.y, 0.06f));
        spawn_explosion_effect(pos, scale, true);
    }
    check_completion();
}

void on_mushroom_sorted(ecs::Entity* entity) {
    if (!entity) return;
    if (!current_level()) {
        entity->mark_deleted();
        return;
    }
    auto* texture = entity->get_checked<texture::OneTextureObject>();
    std::string type = texture ? texture->name : "";
    auto active_it = active_entities.find(type);
    if (active_it != active_entities.end()) {
        active_it->second.erase(entity);
    }
    sorted_counts[type] += 1;
    if (auto* transform = extract_transform(entity)) {
        glm::vec2 pos = transform->pos;
        glm::vec2 scale(std::max(transform->scale_.x, 0.06f), std::max(transform->scale_.y, 0.06f));
        spawn_explosion_effect(pos, scale, false);
    }
    entity->mark_deleted();
    check_completion();
}

void configure_spawners_for_level(const LevelDefinition& level) {
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

void start_level(size_t index) {
    if (parsed_levels.empty()) {
        std::cerr << "No levels parsed" << std::endl;
        return;
    }
    if (index >= parsed_levels.size()) {
        index = 0;
    }
    current_level_index = index;
    level_failed = false;
    level_finished = false;
    reset_active_entities();

    const auto& level = parsed_levels[current_level_index];
    last_played_level_index = current_level_index;
    last_game_status = "Playing " + level.id;
    last_game_success = false;
    collected_counts.clear();
    sorted_counts.clear();
    for (const auto& [type, target] : level.recipe_order) {
        collected_counts[type] = 0;
        sorted_counts[type] = 0;
    }
    scoreboard::init_with_targets(level.recipe_order);
    configure_spawners_for_level(level);
    for (const auto& [type, target] : level.recipe_order) {
        update_scoreboard_for(type);
    }
}

void restart_level() {
    start_level(current_level_index);
}

void advance_level() {
    size_t next_index = current_level_index + 1;
    if (next_index >= parsed_levels.size()) {
        next_index = 0;
    }
    start_level(next_index);
}

bool all_spawns_exhausted() {
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

void check_completion() {
    if (level_finished) return;
    if (!all_spawns_exhausted()) return;

    level_finished = true;
    auto* level = current_level();
    if (!level) return;

    bool success = !level_failed;
    for (const auto& [type, target] : level->recipe_order) {
        if (collected_counts[type] != target) {
            success = false;
            break;
        }
    }

    int total_sorted = 0;
    for (const auto& [type, sorted] : sorted_counts) {
        total_sorted += sorted;
    }

    int total_collected = 0;
    for (const auto& [type, collected] : collected_counts) {
        total_collected += collected;
    }

    if (success) {
        std::cout << "Level " << level->id << " completed successfully!" << std::endl;
        last_game_status = "Completed " + level->id + " (collected " + std::to_string(total_collected) +
                           ", sorted " + std::to_string(total_sorted) + ")";
        last_game_success = true;
    } else {
        std::cout << "Level " << level->id << " failed." << std::endl;
        last_game_status = "Failed " + level->id + " (collected " + std::to_string(total_collected) +
                           ", sorted " + std::to_string(total_sorted) + ")";
        last_game_success = false;
    }

    for (auto& [type, spawner] : spawners_by_type) {
        if (spawner) {
            spawner->enabled = false;
        }
    }
    reset_active_entities();
}

void initialize() {
    parse_levels(file::asset("levels.data"));
    level_finished = true;
    level_failed = false;
    current_level_index = 0;
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

init::CallbackOnStart level_manager_init(&initialize, 7);

} // namespace levels
