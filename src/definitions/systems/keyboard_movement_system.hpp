#pragma once
#include "../../ecs/ecs.hpp"
#include "../components/dynamic_object.hpp"
#include "../components/stateful_object.hpp"
#include "../../definitions/systems/input_system.hpp"
#include "../../definitions/systems/touchscreen_system.hpp"
#include <glm/glm/vec2.hpp>

namespace keyboard_movement {

struct KeyboardMovement : public dynamic::DynamicObject {
    KeyboardMovement(): dynamic::DynamicObject() {}
    virtual ~KeyboardMovement() {
        Component::component_count--;
    }
    float velocity = 0.02f;

    void update() {
        if (scene::is_current_scene_paused()) {
            return;
        }
        ecs::Entity* e = get_entity();
        glm::vec2 d{0.0f, 0.0f};

        if (input::get_button_state(SDL_SCANCODE_A)) {
            d.x -= 0.01;
        }

        if (input::get_button_state(SDL_SCANCODE_D)) {
            d.x += 0.01;
        }

        d += touchscreen::joystick_update.joystick_info;
        states::StatefulObject* a = e->get<states::StatefulObject>();

        double len = glm::length(d);
        if (len == 0) {
            a->set_state("left");
            return;
        }
        double k = velocity / len;
        d *= k;

        transform::TransformObject* t = e->get<transform::TransformObject>();
        t->translate(d);

        if (d.x >= 0) {
            a->set_state("right");
        } else {
            a->set_state("left");
        }
    }
};

} // namespace keyboard_movement