#pragma once

#include "../definitions/components/hidden_object.hpp"
#include "../definitions/components/color_object.hpp"
#include "../definitions/components/geometry_object.hpp"
#include "../definitions/components/dynamic_object.hpp"
#include "../definitions/components/transform_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include "../definitions/components/configurable_object.hpp"
#include "../definitions/components/scene_object.hpp"
#include "../definitions/components/controllable_object.hpp"
#include "../definitions/systems/deferred_system.hpp"
namespace pause_menu {

struct Config {
    glm::vec2 overlay_scale = glm::vec2(2.0f, 2.0f);
    glm::vec2 menu_scale = glm::vec2(0.5f, 0.3f);
    glm::vec2 menu_position = glm::vec2(0.0f, 0.0f);
    glm::vec4 overlay_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.33f);
    glm::vec4 menu_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
} config;

// ImGui configurable parameters
config::Vec2Parameter menu_scale_param("Menu Scale", &config.menu_scale, glm::vec2(0.3f), glm::vec2(0.7f));
config::Vec2Parameter menu_pos_param("Menu Position", &config.menu_position, glm::vec2(-0.5f), glm::vec2(0.5f));
config::Vec4Parameter overlay_color_param("Overlay Color", &config.overlay_color, glm::vec4(0.0f), glm::vec4(1.0f));
config::Vec4Parameter menu_color_param("Menu Color", &config.menu_color, glm::vec4(0.0f), glm::vec4(1.0f));

ecs::Entity* overlay = nullptr;
ecs::Entity* menu_window = nullptr;

void create_pause_menu() {
    overlay = arena::create<ecs::Entity>();
    overlay->add(&geometry::quad);
    overlay->add(arena::create<layers::ConstLayer>(100));
    overlay->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    auto overlay_transform = arena::create<transform::NoRotationTransform>();
    overlay->add(overlay_transform);
    overlay->add(arena::create<shaders::ModelMatrix>());
    overlay->add(arena::create<scene::SceneObject>("main"));
    overlay->add(arena::create<color::OneColor>(config.overlay_color));
    overlay->add(arena::create<hidden::HiddenObject>());
    overlay->get<hidden::HiddenObject>()->hide();
    overlay->bind();

    // Create menu window
    menu_window = arena::create<ecs::Entity>();
    menu_window->add(&geometry::quad);
    menu_window->add(arena::create<layers::ConstLayer>(101));
    menu_window->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    auto menu_transform = arena::create<transform::NoRotationTransform>();
    menu_transform->scale(config.menu_scale);
    menu_transform->translate(config.menu_position);
    menu_window->add(menu_transform);
    menu_window->add(arena::create<shaders::ModelMatrix>());
    menu_window->add(arena::create<scene::SceneObject>("main"));
    menu_window->add(arena::create<color::OneColor>(config.menu_color));
    menu_window->add(arena::create<hidden::HiddenObject>());
    menu_window->add(arena::create<texture::OneTextureObject>("pause_menu"));
    menu_window->get<hidden::HiddenObject>()->hide();

    menu_window->bind();
}

ecs::Entity* digit_sprite = nullptr;

void show_digit_callback(int digit) {
    LOG_IF(logger::enable_pause_menu_logging, "show digit callback " << digit);
    if (digit_sprite != nullptr) {
        digit_sprite->mark_deleted();
    }
    digit_sprite = nullptr;
    if (digit == 0) {
        auto overlay_hidden = overlay->get<hidden::HiddenObject>();
        overlay_hidden->hide();
        scene::toggle_pause();
        return;
    }
    digit_sprite = arena::create<ecs::Entity>();
    digit_sprite->add(&geometry::quad);
    digit_sprite->add(arena::create<layers::ConstLayer>(102));
    digit_sprite->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    digit_sprite->add(arena::create<scene::SceneObject>("main"));
    digit_sprite->add(arena::create<color::OneColor>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    auto digit_transform = arena::create<transform::NoRotationTransform>();
    digit_transform->translate(glm::vec3(0.0f, 0.0f, 0.0f));
    digit_transform->scale(glm::vec3(0.3f, 0.3f, 1.0f));
    digit_sprite->add(digit_transform);
    digit_sprite->add(arena::create<shaders::ModelMatrix>());
    digit_sprite->add(arena::create<texture::OneTextureObject>(std::string("digits_") + std::to_string(digit)));
    digit_sprite->bind();
    deferred::fire_deferred([=]() {
        show_digit_callback(digit - 1);
    }, 1000);
}

struct TogglePauseMenu: public input::ControllableObject {
    TogglePauseMenu(): input::ControllableObject() {}
    void handle_user_action(SDL_Event e) {
        if (e.type != SDL_KEYDOWN) return;
        if (e.key.keysym.scancode == SDL_SCANCODE_P) {
            auto overlay_hidden = overlay->get<hidden::HiddenObject>();
            auto menu_hidden = menu_window->get<hidden::HiddenObject>();
            auto scene_object = overlay->get<scene::SceneObject>();
            if (scene_object->get_scene()->is_paused_state()) {
                LOG_IF(logger::enable_pause_menu_logging, "show pause menu");
                auto menu_hidden = menu_window->get<hidden::HiddenObject>();
                menu_hidden->hide();
                show_digit_callback(3);
            } else {
                if (!overlay_hidden->is_visible()) {
                    overlay_hidden->show();
                    menu_hidden->show();
                }
                scene::toggle_pause();
            }
        }
    }
};
TogglePauseMenu pause_menu_toggle_controllable;

void init() {
    std::cout << "init pause menu\n";
    create_pause_menu();
}

init::CallbackOnStart pause_menu_init(&init, 6);
} // namespace pause_menu