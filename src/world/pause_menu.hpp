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
#include "../definitions/components/text_object.hpp"
#include "../definitions/components/layered_object.hpp"
#include "../definitions/components/textured_object.hpp"
#include "../definitions/systems/deferred_system.hpp"
#include "../declarations/text_system.hpp"
#include "../declarations/color_system.hpp"
#include "level_manager.hpp"
#include "lives.hpp"
#include <utility>
namespace pause_menu {

struct Config {
    glm::vec2 overlay_scale = glm::vec2(2.0f, 2.0f);
    glm::vec2 menu_scale = glm::vec2(0.15f, 0.15f);
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
ecs::Entity* restart_button = nullptr;
ecs::Entity* restart_text = nullptr;
hidden::HiddenObject* restart_button_hidden = nullptr;
hidden::HiddenObject* restart_text_hidden = nullptr;
transform::NoRotationTransform* restart_button_transform = nullptr;
transform::NoRotationTransform* restart_text_transform = nullptr;
text::TextObject* restart_text_object = nullptr;

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

    restart_button = arena::create<ecs::Entity>();
    restart_button->add(&geometry::quad);
    restart_button->add(arena::create<layers::ConstLayer>(102));
    restart_button->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    restart_button_transform = arena::create<transform::NoRotationTransform>();
    restart_button_transform->scale(glm::vec2(0.25f, 0.08f));
    restart_button_transform->translate(config.menu_position + glm::vec2(0.0f, -0.2f));
    restart_button->add(restart_button_transform);
    restart_button->add(arena::create<shaders::ModelMatrix>());
    restart_button->add(arena::create<scene::SceneObject>("main"));
    restart_button->add(arena::create<color::OneColor>(glm::vec4(0.15f, 0.15f, 0.15f, 0.95f)));
    restart_button_hidden = arena::create<hidden::HiddenObject>();
    restart_button->add(restart_button_hidden);
    restart_button_hidden->hide();
    restart_button->bind();

    restart_text = arena::create<ecs::Entity>();
    restart_text->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));
    restart_text_transform = arena::create<transform::NoRotationTransform>();
    restart_text_transform->scale(glm::vec2(0.05f, 0.05f));
    restart_text_transform->translate(config.menu_position + glm::vec2(0.0f, -0.2f));
    restart_text->add(restart_text_transform);
    restart_text->add(arena::create<shaders::ModelMatrix>());
    restart_text->add(arena::create<layers::ConstLayer>(103));
    restart_text->add(arena::create<scene::SceneObject>("main"));
    restart_text->add(&color::white);
    restart_text->add(text::text_texture);
    restart_text_object = arena::create<text::TextObject>("Restart");
    restart_text->add(restart_text_object);
    restart_text_hidden = arena::create<hidden::HiddenObject>();
    restart_text->add(restart_text_hidden);
    restart_text_hidden->hide();
    restart_text->bind();
    text::text_loader.init(restart_text_object);
}

ecs::Entity* digit_sprite = nullptr;

void set_restart_visibility(bool visible) {
    if (!restart_button_hidden || !restart_text_hidden) return;
    if (visible) {
        restart_button_hidden->show();
        restart_text_hidden->show();
    } else {
        restart_button_hidden->hide();
        restart_text_hidden->hide();
    }
}

std::pair<glm::vec2, glm::vec2> restart_button_bounds() {
    if (!restart_button_transform) {
        return {glm::vec2(0.0f), glm::vec2(0.0f)};
    }
    glm::vec2 center = restart_button_transform->get_pos();
    glm::vec2 half_extent = restart_button_transform->scale_;
    return {center - half_extent, center + half_extent};
}

void show_digit_callback(int digit) {
    LOG_IF(logger::enable_pause_menu_logging, "show digit callback " << digit);
    if (digit_sprite != nullptr) {
        digit_sprite->mark_deleted();
    }
    digit_sprite = nullptr;
    set_restart_visibility(false);
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
    digit_transform->scale(glm::vec3(0.15f, 0.15f, 1.0f));
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
                set_restart_visibility(false);
                show_digit_callback(3);
            } else {
                if (!overlay_hidden->is_visible()) {
                    overlay_hidden->show();
                    menu_hidden->show();
                    set_restart_visibility(true);
                }
                scene::toggle_pause();
            }
        }
    }
};
TogglePauseMenu pause_menu_toggle_controllable;

struct RestartButtonHandler : public input::ControllableObject {
    RestartButtonHandler() : input::ControllableObject() {}

    void handle_user_action(SDL_Event e) override {
        if (e.type != SDL_MOUSEBUTTONDOWN) return;
        if (e.button.button != SDL_BUTTON_LEFT) return;
        if (!scene::is_current_scene_paused()) return;
        if (!restart_button_hidden || !restart_button_hidden->is_visible()) return;

        SDL_Window* window = SDL_GetWindowFromID(e.button.windowID);
        int width = 0;
        int height = 0;
        if (window) {
            SDL_GetWindowSize(window, &width, &height);
        }
        if (width == 0 || height == 0) {
            return;
        }

        float norm_x = (static_cast<float>(e.button.x) / static_cast<float>(width)) * 2.0f - 1.0f;
        float norm_y = 1.0f - (static_cast<float>(e.button.y) / static_cast<float>(height)) * 2.0f;
        auto [min_bound, max_bound] = restart_button_bounds();
        if (norm_x < min_bound.x || norm_x > max_bound.x || norm_y < min_bound.y || norm_y > max_bound.y) {
            return;
        }

        if (digit_sprite) {
            digit_sprite->mark_deleted();
            digit_sprite = nullptr;
        }
        levels::restart_level();
        lives::reset_lives();
        set_restart_visibility(false);
        auto overlay_hidden = overlay->get<hidden::HiddenObject>();
        auto menu_hidden = menu_window->get<hidden::HiddenObject>();
        overlay_hidden->hide();
        menu_hidden->hide();
        if (scene::is_current_scene_paused()) {
            scene::toggle_pause();
        }
    }
};
RestartButtonHandler restart_button_handler;

void init() {
    std::cout << "init pause menu\n";
    create_pause_menu();
}

init::CallbackOnStart pause_menu_init(&init, 6);
} // namespace pause_menu