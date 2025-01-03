#pragma once
#include "../definitions/components/text_object.hpp"
#include "../definitions/systems/scene_system.hpp"
#include "../declarations/text_system.hpp"
#include "../declarations/color_system.hpp"
#include "../definitions/components/configurable_object.hpp"

namespace lives {

struct Config {
    glm::vec2 heart_scale = glm::vec2(0.08f, 0.08f);
    glm::vec2 base_position = glm::vec2(0.6f, 0.7f);
    float horizontal_spacing = 0.10f;
} config;

const int MAX_LIVES = 3;
int current_lives = MAX_LIVES;
ecs::Entity* heart_icons[MAX_LIVES];

// ImGui configurable parameters
config::Vec2Parameter heart_scale_param("Heart Scale", &config.heart_scale, glm::vec2(0.05f), glm::vec2(0.1f));
config::Vec2Parameter base_pos_param("Hearts Base Position", &config.base_position, glm::vec2(0.5f, 0.6f), glm::vec2(0.8f, 0.8f));
config::FloatParameter spacing_param("Horizontal Spacing", &config.horizontal_spacing, 0.07f, 0.13f);

void remove_life() {
    if (current_lives <= 0) return;
    heart_icons[current_lives - 1]->mark_deleted();
    current_lives--;
}

void reset_lives() {
    LOG_IF(logger::enable_lives_system_logging, "reset lives\n");
    current_lives = MAX_LIVES;
    for (int i = 0; i < MAX_LIVES; i++) {
        if (heart_icons[i]) {
            heart_icons[i]->mark_deleted();
        }
        heart_icons[i] = arena::create<ecs::Entity>();
        auto transform = arena::create<transform::NoRotationTransform>();
        
        transform->scale(config.heart_scale);
        glm::vec2 position = config.base_position + glm::vec2(config.horizontal_spacing * i, 0.0f);
        transform->translate(position);
        
        heart_icons[i]->add(&geometry::quad);
        heart_icons[i]->add(arena::create<layers::ConstLayer>(5));
        heart_icons[i]->add(arena::create<texture::OneTextureObject>("heart"));
        heart_icons[i]->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
        heart_icons[i]->add(transform);
        heart_icons[i]->add(arena::create<shaders::ModelMatrix>());
        heart_icons[i]->add(arena::create<scene::SceneObject>("main"));
        heart_icons[i]->bind();
    }
}

void init() {
    LOG_IF(logger::enable_lives_system_logging, "init lives system\n");
    reset_lives();
}

init::CallbackOnStart lives_init(&init, 6);

} // namespace lives 