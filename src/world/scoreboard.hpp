#pragma once

#include "../definitions/components/transform_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include "../definitions/components/textured_object.hpp"
#include "../definitions/components/layered_object.hpp"
#include "../definitions/systems/scene_system.hpp"
#include "../declarations/color_system.hpp"
#include "../definitions/components/configurable_object.hpp"
#include "../geometry/quad.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace scoreboard {

struct Config {
    glm::vec2 icon_scale = glm::vec2(0.03f, 0.02f);
    glm::vec2 score_scale = glm::vec2(0.035f, 0.02f);
    glm::vec2 base_icon_position = glm::vec2(0.55f, 0.45f);
    glm::vec2 base_score_position = glm::vec2(0.72f, 0.43f);
    float vertical_spacing_1 = 0.08f;
    float vertical_spacing_2 = 0.08f;
    float char_spacing = 0.08f;
} config;

struct Entry {
    std::string name;
    int current = 0;
    int target = 0;
    ecs::Entity* icon = nullptr;
    size_t row_index = 0;
    std::vector<ecs::Entity*> digits;
};

std::vector<Entry> entries;
std::unordered_map<std::string, size_t> entry_index;
std::vector<std::pair<std::string, int>> current_recipe;

config::Vec2Parameter icon_scale_param("Icon Scale", &config.icon_scale, glm::vec2(0.01f), glm::vec2(0.21f));
config::Vec2Parameter score_scale_param("Score Scale", &config.score_scale, glm::vec2(0.01f), glm::vec2(0.21f));
config::Vec2Parameter icon_pos_param("Icon Base Position", &config.base_icon_position, glm::vec2(0.5f, 0.3f), glm::vec2(0.8f, 0.8f));
config::Vec2Parameter score_pos_param("Score Base Position", &config.base_score_position, glm::vec2(0.6f, 0.3f), glm::vec2(0.9f, 0.8f));
config::FloatParameter spacing_param("Vertical Spacing", &config.vertical_spacing_1, 0.07f, 0.13f);
config::FloatParameter spacing_param_2("Vertical Spacing 2", &config.vertical_spacing_2, 0.07f, 0.13f);
config::FloatParameter char_spacing_param("Character Spacing", &config.char_spacing, 0.01f, 0.12f);

void destroy_digits(Entry& entry) {
    for (auto* digit : entry.digits) {
        if (digit) {
            digit->mark_deleted();
        }
    }
    entry.digits.clear();
}

void destroy_entry(Entry& entry) {
    if (entry.icon) {
        entry.icon->mark_deleted();
        entry.icon = nullptr;
    }
    destroy_digits(entry);
}

void clear_entries() {
    for (auto& entry : entries) {
        destroy_entry(entry);
    }
    entries.clear();
    entry_index.clear();
}

std::string texture_name_for_char(char ch) {
    if (ch >= '0' && ch <= '9') {
        return std::string("digits_") + ch;
    }
    if (ch == '/') {
        return "slash";
    }
    return "digits_0";
}

glm::vec2 score_base_position(size_t row_index) {
    return config.base_score_position + glm::vec2(0.0f, config.vertical_spacing_2 * static_cast<float>(row_index));
}

void update_entry_digits(Entry& entry) {
    destroy_digits(entry);
    std::string formatted = std::to_string(entry.current) + "/" + std::to_string(entry.target);
    if (formatted.empty()) {
        return;
    }

    glm::vec2 base_center = score_base_position(entry.row_index);
    float char_width = config.score_scale.x * 2.0f;
    float total_width = char_width * static_cast<float>(formatted.size());
    if (formatted.size() > 1) {
        total_width += config.char_spacing * static_cast<float>(formatted.size() - 1);
    }
    float start_x = base_center.x - total_width * 0.5f;

    for (size_t i = 0; i < formatted.size(); ++i) {
        std::string texture_name = texture_name_for_char(formatted[i]);
        auto digit_entity = arena::create<ecs::Entity>();
        digit_entity->add(&geometry::quad);
        digit_entity->add(arena::create<layers::ConstLayer>(5));
        digit_entity->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
        auto transform = arena::create<transform::NoRotationTransform>();
        transform->scale(config.score_scale);
        float center_x = start_x + (char_width * 0.5f) + static_cast<float>(i) * (char_width + config.char_spacing);
        transform->translate(glm::vec2(center_x, base_center.y));
        digit_entity->add(transform);
        digit_entity->add(arena::create<shaders::ModelMatrix>());
        digit_entity->add(arena::create<texture::OneTextureObject>(texture_name));
        digit_entity->add(&color::white);
        digit_entity->add(arena::create<scene::SceneObject>("main"));
        digit_entity->bind();
        entry.digits.push_back(digit_entity);
    }
}

void create_entry_visuals(Entry& entry, size_t index) {
    auto transform_icon = arena::create<transform::NoRotationTransform>();

    transform_icon->scale(config.icon_scale);

    glm::vec2 icon_position = config.base_icon_position + glm::vec2(0.0f, config.vertical_spacing_1 * static_cast<float>(index));

    transform_icon->translate(icon_position);

    entry.icon = arena::create<ecs::Entity>();
    entry.icon->add(&geometry::quad);
    entry.icon->add(arena::create<layers::ConstLayer>(5));
    entry.icon->add(arena::create<texture::OneTextureObject>(entry.name));
    entry.icon->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    entry.icon->add(transform_icon);
    entry.icon->add(arena::create<shaders::ModelMatrix>());
    entry.icon->add(arena::create<scene::SceneObject>("main"));
    entry.icon->bind();

    entry.row_index = index;
    update_entry_digits(entry);
}

void rebuild_entries() {
    clear_entries();
    entries.reserve(current_recipe.size());
    for (size_t i = 0; i < current_recipe.size(); ++i) {
        Entry entry;
        entry.name = current_recipe[i].first;
        entry.target = current_recipe[i].second;
        entry.current = 0;
        create_entry_visuals(entry, i);
        entry_index[entry.name] = entries.size();
        entries.push_back(entry);
    }
}

config::ButtonParameter reset_button("Reset", []() {
    rebuild_entries();
});

void init_with_targets(const std::vector<std::pair<std::string, int>>& recipe) {
    current_recipe = recipe;
    rebuild_entries();
}

void update_score(const std::string& name, int new_score, int target) {
    auto it = entry_index.find(name);
    if (it == entry_index.end()) return;
    auto& entry = entries[it->second];
    entry.current = new_score;
    entry.target = target;
    update_entry_digits(entry);
}

void init() {
    entries.clear();
    entry_index.clear();
    current_recipe.clear();
}

init::CallbackOnStart scoreboard_init(&init, 6);

} // namespace scoreboard
