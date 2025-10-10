#pragma once

#include "../definitions/components/text_object.hpp"
#include "../definitions/components/transform_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include "../definitions/components/textured_object.hpp"
#include "../definitions/components/layered_object.hpp"
#include "../definitions/systems/scene_system.hpp"
#include "../declarations/text_system.hpp"
#include "../declarations/color_system.hpp"
#include "../definitions/components/configurable_object.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace scoreboard {

struct Config {
    glm::vec2 icon_scale = glm::vec2(0.05f, 0.05f);
    glm::vec2 score_scale = glm::vec2(0.05f, 0.05f);
    glm::vec2 base_icon_position = glm::vec2(0.6f, 0.37f);
    glm::vec2 base_score_position = glm::vec2(0.75f, 0.35f);
    float vertical_spacing_1 = 0.10f;
    float vertical_spacing_2 = 0.10f;
} config;

struct Entry {
    std::string name;
    int current = 0;
    int target = 0;
    ecs::Entity* icon = nullptr;
    ecs::Entity* text_entity = nullptr;
    text::TextObject* text_object = nullptr;
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

void destroy_entry(Entry& entry) {
    if (entry.icon) {
        entry.icon->mark_deleted();
        entry.icon = nullptr;
    }
    if (entry.text_entity) {
        entry.text_entity->mark_deleted();
        entry.text_entity = nullptr;
    }
    entry.text_object = nullptr;
}

void clear_entries() {
    for (auto& entry : entries) {
        destroy_entry(entry);
    }
    entries.clear();
    entry_index.clear();
}

void update_entry_text(Entry& entry) {
    if (!entry.text_entity) return;
    if (entry.text_object) {
        entry.text_entity->detach(entry.text_object);
    }
    auto geom = entry.text_entity->get_checked<text::TextGeometry>();
    if (geom) {
        entry.text_entity->detach(geom);
    }
    entry.text_object = arena::create<text::TextObject>(std::to_string(entry.current) + "/" + std::to_string(entry.target));
    entry.text_entity->add(entry.text_object);
    text::text_loader.init(entry.text_object);
}

void create_entry_visuals(Entry& entry, size_t index) {
    auto transform_icon = arena::create<transform::NoRotationTransform>();
    auto transform_score = arena::create<transform::NoRotationTransform>();

    transform_icon->scale(config.icon_scale);
    transform_score->scale(config.score_scale);

    glm::vec2 icon_position = config.base_icon_position + glm::vec2(0.0f, config.vertical_spacing_1 * static_cast<float>(index));
    glm::vec2 score_position = config.base_score_position + glm::vec2(0.0f, config.vertical_spacing_2 * static_cast<float>(index));

    transform_icon->translate(icon_position);
    transform_score->translate(score_position);

    entry.icon = arena::create<ecs::Entity>();
    entry.icon->add(&geometry::quad);
    entry.icon->add(arena::create<layers::ConstLayer>(5));
    entry.icon->add(arena::create<texture::OneTextureObject>(entry.name));
    entry.icon->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    entry.icon->add(transform_icon);
    entry.icon->add(arena::create<shaders::ModelMatrix>());
    entry.icon->add(arena::create<scene::SceneObject>("main"));
    entry.icon->bind();

    entry.text_entity = arena::create<ecs::Entity>();
    entry.text_entity->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    entry.text_entity->add(transform_score);
    entry.text_entity->add(arena::create<shaders::ModelMatrix>());
    entry.text_entity->add(arena::create<layers::ConstLayer>(5));
    entry.text_entity->add(arena::create<scene::SceneObject>("main"));
    entry.text_entity->add(&color::white);
    entry.text_entity->add(text::text_texture);
    entry.text_entity->bind();
    update_entry_text(entry);
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
    update_entry_text(entry);
}

void init() {
    entries.clear();
    entry_index.clear();
    current_recipe.clear();
}

init::CallbackOnStart scoreboard_init(&init, 6);

} // namespace scoreboard
