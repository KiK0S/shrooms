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
    glm::vec2 last_movement{0.0f, 0.0f};  // Store last movement direction
    bool enabled = true;

    void update() {
        if (scene::is_current_scene_paused()) {
            return;
        }
        if (!enabled) {
            last_movement = {0.0f, 0.0f};
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

        double len = glm::length(d);
        if (len == 0) {
            last_movement = {0.0f, 0.0f};
            return;
        }
        double k = velocity / len;
        d *= k;

        transform::TransformObject* t = e->get<transform::TransformObject>();
        t->translate(d);
        if (t->get_pos().x < -1.0f) {
            t->translate(-d);
        }
        if (t->get_pos().x > 1.0f) {
            t->translate(-d);
        }

        last_movement = d;  // Store the movement
    }
};

// New sprite state update component
struct SpriteStateUpdate : public dynamic::DynamicObject {
    SpriteStateUpdate(KeyboardMovement* movement): movement(movement), dynamic::DynamicObject(1) {}
    virtual ~SpriteStateUpdate() {
        Component::component_count--;
    }

    void update() {
        if (scene::is_current_scene_paused()) {
            return;
        }
        
        auto stateful = get_entity()->get<states::StatefulObject>();
        if (!stateful) return;

        if (movement->last_movement.x == 0) {
            stateful->set_state("left");
        } else if (movement->last_movement.x >= 0) {
            stateful->set_state("right");
        } else {
            stateful->set_state("left");
        }
    }

    KeyboardMovement* movement;
};

} // namespace keyboard_movement
