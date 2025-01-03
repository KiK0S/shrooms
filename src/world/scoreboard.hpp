#pragma once
#include "../definitions/components/text_object.hpp"
#include "../definitions/systems/scene_system.hpp"
#include "../declarations/text_system.hpp"
#include "../declarations/color_system.hpp"
#include "../definitions/components/configurable_object.hpp"

namespace scoreboard {

struct Config {
    glm::vec2 icon_scale = glm::vec2(0.05f, 0.05f);
    glm::vec2 score_scale = glm::vec2(0.05f, 0.05f);
    glm::vec2 base_icon_position = glm::vec2(0.6f, 0.37f);
    glm::vec2 base_score_position = glm::vec2(0.75f, 0.35f);
    float vertical_spacing_1 = 0.10f;
    float vertical_spacing_2 = 0.10f;
} config;

ecs::Entity score_display;
ecs::Entity* mushroom_icons[3];
std::string mushroom_names[3] = {"mukhomor", "lisi4ka", "borovik"};
ecs::Entity* mushroom_scores[3];
text::TextObject* mushroom_scores_text[3];
std::map<std::string, int> eaten_mushrooms;


// ImGui configurable parameters
config::Vec2Parameter icon_scale_param("Icon Scale", &config.icon_scale, glm::vec2(0.01f), glm::vec2(0.21f));
config::Vec2Parameter score_scale_param("Score Scale", &config.score_scale, glm::vec2(0.01f), glm::vec2(0.21f));
config::Vec2Parameter icon_pos_param("Icon Base Position", &config.base_icon_position, glm::vec2(0.5f, 0.3f), glm::vec2(0.8f, 0.8f));
config::Vec2Parameter score_pos_param("Score Base Position", &config.base_score_position, glm::vec2(0.6f, 0.3f), glm::vec2(0.9f, 0.8f));
config::FloatParameter spacing_param("Vertical Spacing", &config.vertical_spacing_1, 0.07f, 0.13f);
config::FloatParameter spacing_param_2("Vertical Spacing 2", &config.vertical_spacing_2, 0.07f, 0.13f);
config::ButtonParameter reset_button("Reset", []() {
    for (int i = 0; i < 3; i++) {
        mushroom_icons[i]->mark_deleted();
        mushroom_scores[i]->mark_deleted();
    }
    for (int i = 0; i < 3; i++) {
        mushroom_icons[i] = arena::create<ecs::Entity>();
        mushroom_scores[i] = arena::create<ecs::Entity>();
        auto transform_icon = arena::create<transform::NoRotationTransform>();
        auto transform_score = arena::create<transform::NoRotationTransform>();
        
        transform_icon->scale(config.icon_scale);
        transform_score->scale(config.score_scale);
        
        glm::vec2 icon_position = config.base_icon_position + glm::vec2(0.0f, config.vertical_spacing_1 * i);
        glm::vec2 score_position = config.base_score_position + glm::vec2(0.0f, config.vertical_spacing_2 * i);
        
        transform_icon->translate(icon_position);
        transform_score->translate(score_position);
        
        eaten_mushrooms[mushroom_names[i]] = 0;
        mushroom_icons[i]->add(&geometry::quad);
        mushroom_icons[i]->add(arena::create<layers::ConstLayer>(5));
        mushroom_icons[i]->add(arena::create<texture::OneTextureObject>(file::asset(mushroom_names[i])));
        mushroom_icons[i]->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
        mushroom_icons[i]->add(transform_icon);
        mushroom_icons[i]->add(arena::create<shaders::ModelMatrix>());
        mushroom_icons[i]->add(arena::create<scene::SceneObject>("main"));
        mushroom_icons[i]->bind();

        mushroom_scores_text[i] = arena::create<text::TextObject>(std::to_string(eaten_mushrooms[mushroom_names[i]]));
        mushroom_scores[i]->add(mushroom_scores_text[i]);
        mushroom_scores[i]->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
        mushroom_scores[i]->add(transform_score);
        mushroom_scores[i]->add(arena::create<shaders::ModelMatrix>());
        mushroom_scores[i]->add(arena::create<layers::ConstLayer>(5));
        mushroom_scores[i]->add(arena::create<scene::SceneObject>("main"));
        mushroom_scores[i]->add(&color::white);
        mushroom_scores[i]->add(text::text_texture);
        mushroom_scores[i]->bind();
        text::text_loader.init(mushroom_scores[i]->get_checked<text::TextObject>());
    }
});


void update_score(std::string name, int new_score) {
    eaten_mushrooms[name] = new_score;
    int id = 0;
    for (int i = 0; i < 3; i++) {
        if (mushroom_names[i] == name) {
            id = i;
            break;
        }
    }
    mushroom_scores[id]->detach(mushroom_scores_text[id]);
    auto geom = mushroom_scores[id]->get_checked<text::TextGeometry>();
    mushroom_scores[id]->detach(geom);
    mushroom_scores_text[id] = arena::create<text::TextObject>(std::to_string(new_score));
    mushroom_scores[id]->add(mushroom_scores_text[id]);
    text::text_loader.init(mushroom_scores_text[id]);
}

void init() {
    std::cout << "init scoreboard\n";
    for (int i = 0; i < 3; i++) {
        mushroom_icons[i] = arena::create<ecs::Entity>();
        mushroom_scores[i] = arena::create<ecs::Entity>();
        auto transform_icon = arena::create<transform::NoRotationTransform>();
        auto transform_score = arena::create<transform::NoRotationTransform>();
        
        transform_icon->scale(config.icon_scale);
        transform_score->scale(config.score_scale);
        
        glm::vec2 icon_position = config.base_icon_position + glm::vec2(0.0f, config.vertical_spacing_1 * i);
        glm::vec2 score_position = config.base_score_position + glm::vec2(0.0f, config.vertical_spacing_2 * i);
        
        transform_icon->translate(icon_position);
        transform_score->translate(score_position);
        
        eaten_mushrooms[mushroom_names[i]] = 0;
        mushroom_icons[i]->add(&geometry::quad);
        mushroom_icons[i]->add(arena::create<layers::ConstLayer>(5));
        mushroom_icons[i]->add(arena::create<texture::OneTextureObject>(file::asset(mushroom_names[i])));
        mushroom_icons[i]->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
        mushroom_icons[i]->add(transform_icon);
        mushroom_icons[i]->add(arena::create<shaders::ModelMatrix>());
        mushroom_icons[i]->add(arena::create<scene::SceneObject>("main"));
        mushroom_icons[i]->bind();

        mushroom_scores_text[i] = arena::create<text::TextObject>(std::to_string(eaten_mushrooms[mushroom_names[i]]));
        mushroom_scores[i]->add(mushroom_scores_text[i]);
        mushroom_scores[i]->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
        mushroom_scores[i]->add(transform_score);
        mushroom_scores[i]->add(arena::create<shaders::ModelMatrix>());
        mushroom_scores[i]->add(arena::create<layers::ConstLayer>(5));
        mushroom_scores[i]->add(arena::create<scene::SceneObject>("main"));
        mushroom_scores[i]->add(&color::white);
        mushroom_scores[i]->add(text::text_texture);
        mushroom_scores[i]->bind();
        text::text_loader.init(mushroom_scores[i]->get_checked<text::TextObject>());
    }

}

init::CallbackOnStart scoreboard_init(&init, 6);

} // namespace scoreboard
