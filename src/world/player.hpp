#pragma once
#include "../definitions/components/sprite_object.hpp"
#include "../definitions/components/collider_object.hpp"
#include "../definitions/systems/keyboard_movement_system.hpp"
#include "../definitions/systems/scene_system.hpp"
#include "../definitions/components/blinking_object.hpp"
#include "../definitions/components/hidden_object.hpp"
#include "../definitions/components/layered_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include "../declarations/color_system.hpp"
#include "../declarations/shader_system.hpp"
#include "../geometry/quad.hpp"
#include <glm/glm.hpp>
#include "level_manager.hpp"

namespace player {

keyboard_movement::KeyboardMovement player_control;
keyboard_movement::SpriteStateUpdate sprite_state_update(&player_control);
scene::SceneObject player_scene("main");
ecs::Entity player;
config::FloatParameter player_speed("Player speed", &player_control.velocity, 0.0f, 0.05f);
blinking::BlinkingObject player_blink;
hidden::HiddenObject player_hidden;

transform::NoRotationTransform player_transform;
transform::NoRotationTransform sprite_transform;
transform::NoRotationTransform trigger_transform;

hidden::HiddenObject projectile_hidden;
transform::NoRotationTransform projectile_transform;

enum class ProjectileState {
    Idle,
    Outbound,
    Returning
};

struct BoneFootProjectile : public dynamic::DynamicObject {
    BoneFootProjectile(): dynamic::DynamicObject() {}

    void update() override {
        if (scene::is_current_scene_paused()) {
            return;
        }

        bool fire_pressed = input::get_button_state(SDL_SCANCODE_W);
        if (state == ProjectileState::Idle && fire_pressed && !fire_pressed_last_frame) {
            glm::vec2 start_pos = player_transform.get_pos() + glm::vec2(0.0f, 0.25f);
            fire_from_origin(start_pos);
        }

        fire_pressed_last_frame = fire_pressed;

        if (state == ProjectileState::Idle) {
            auto* hidden_comp = get_entity()->get<hidden::HiddenObject>();
            if (hidden_comp && hidden_comp->is_visible()) {
                hidden_comp->hide();
            }
            return;
        }

        auto* hidden_comp = get_entity()->get<hidden::HiddenObject>();
        if (hidden_comp && !hidden_comp->is_visible()) {
            hidden_comp->show();
        }

        auto* transform = get_entity()->get<transform::NoRotationTransform>();
        if (!transform) {
            return;
        }

        transform->pos += direction * speed;

        if (state == ProjectileState::Outbound) {
            if (transform->pos.y >= max_height) {
                start_return();
            }
        } else if (state == ProjectileState::Returning) {
            glm::vec2 to_origin = origin - transform->pos;
            if (glm::length(to_origin) <= speed) {
                finish();
                return;
            }
            if (glm::length(to_origin) > 0.0f) {
                direction = glm::normalize(to_origin);
            }
        }
    }

    void fire_from_origin(const glm::vec2& start_pos) {
        origin = start_pos;
        direction = glm::vec2(0.0f, 1.0f);
        state = ProjectileState::Outbound;
        player_control.enabled = false;
        player_control.last_movement = {0.0f, 0.0f};
        auto* transform = get_entity()->get<transform::NoRotationTransform>();
        if (transform) {
            transform->pos = start_pos;
        }
        if (auto* hidden_comp = get_entity()->get<hidden::HiddenObject>()) {
            hidden_comp->show();
        }
    }

    void start_return() {
        if (state == ProjectileState::Idle) {
            return;
        }
        state = ProjectileState::Returning;
        auto* transform = get_entity()->get<transform::NoRotationTransform>();
        if (!transform) {
            return;
        }
        glm::vec2 to_origin = origin - transform->pos;
        if (glm::length(to_origin) > 0.0f) {
            direction = glm::normalize(to_origin);
        } else {
            finish();
        }
    }

    void finish() {
        state = ProjectileState::Idle;
        direction = glm::vec2(0.0f, 1.0f);
        player_control.enabled = true;
        player_control.last_movement = {0.0f, 0.0f};
        origin = player_transform.get_pos() + glm::vec2(0.0f, 0.25f);
        auto* transform = get_entity()->get<transform::NoRotationTransform>();
        if (transform) {
            transform->pos = origin;
        }
        if (auto* hidden_comp = get_entity()->get<hidden::HiddenObject>()) {
            hidden_comp->hide();
        }
        fire_pressed_last_frame = false;
    }

    void handle_hit(ecs::Entity* mushroom) {
        if (!mushroom) {
            return;
        }
        if (state != ProjectileState::Outbound) {
            return;
        }
        levels::on_mushroom_sorted(mushroom);
        start_return();
    }

    bool fire_pressed_last_frame = false;
    float speed = 0.03f;
    float max_height = 0.9f;
    glm::vec2 origin{0.0f, 0.0f};
    glm::vec2 direction{0.0f, 1.0f};
    ProjectileState state = ProjectileState::Idle;
};

BoneFootProjectile projectile_logic;
collision::TriggerObject projectile_trigger("bone_projectile_handler",
    [](ecs::Entity*, collision::ColliderObject* collider) {
        auto* entity = collider ? collider->get_entity() : nullptr;
        projectile_logic.handle_hit(entity);
    }
);
ecs::Entity projectile_entity;

sprite::AnimatedSprite player_sprite("witch", {-1.0f, -1.0f}, {1.0f, 1.0f}, 2, {
    {"right", {0.2, 0.2}},
    {"left", {0.2, 0.2}}
});

ecs::Entity player_sprite_entity;
ecs::Entity player_trigger_entity;

collision::TriggerObject player_trigger("mushroom_catch_handler",
    [](ecs::Entity* _player, collision::ColliderObject* mushroom) {
        auto* entity = mushroom->get_entity();
        auto* texture = entity->get_checked<texture::OneTextureObject>();
        std::string mushroom_type = texture ? texture->name : "";
        LOG_IF(logger::enable_collision_system_logging, "Collected mushroom " << mushroom_type << "!");
        levels::on_mushroom_caught(mushroom_type, entity);
        entity->mark_deleted();
    }
);

void reset_for_new_level() {
    player_transform.pos = glm::vec2(0.0f, -0.6f);
    player_control.enabled = true;
    player_control.last_movement = {0.0f, 0.0f};
    projectile_logic.finish();
}

void init() {
    std::cout << "init player\n";
    player_transform.scale(glm::vec2(0.1f, 0.1f));
    player_transform.translate(glm::vec2(0.0f, -0.6f));
    player.add(&player_control)
          .add(&player_speed)
          .add(&player_transform)
          .bind();

    player_sprite_entity.add(&player_sprite)
                       .add(&player_scene)
                       .add(&player_blink)
                       .add(&player_hidden)
                       .add(&sprite_state_update)
                       .set_parent(&player)
                       .bind();

    trigger_transform.scale_ = glm::vec2(0.7f, 0.2f);
    trigger_transform.translate(glm::vec2(-0.1f, 1.0f));
    player_trigger_entity.add(&trigger_transform)
                        .add(&player_trigger)
                        .add(&geometry::quad)
                        // .add(&color::red)
                        // .add(arena::create<layers::ConstLayer>(1))
                        // .add(arena::create<shaders::ModelMatrix>())
                        // .add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program))
                        // .add(arena::create<scene::SceneObject>("main"))
                        .set_parent(&player)
                        .bind();

    projectile_transform.scale(glm::vec2(0.025f, 0.25f));
    projectile_transform.pos = player_transform.get_pos() + glm::vec2(0.0f, 0.25f);

    auto projectile_layer = arena::create<layers::ConstLayer>(6);
    auto projectile_program = arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program);
    auto projectile_model = arena::create<shaders::ModelMatrix>();
    auto projectile_color = arena::create<color::OneColor>(glm::vec4(0.95f, 0.95f, 0.95f, 1.0f));

    projectile_entity.add(&geometry::quad)
                     .add(projectile_layer)
                     .add(projectile_program)
                     .add(&projectile_transform)
                     .add(projectile_model)
                     .add(projectile_color)
                     .add(&projectile_hidden)
                     .add(&projectile_logic)
                     .add(&projectile_trigger)
                     .add(arena::create<scene::SceneObject>("main"))
                     .bind();

    projectile_hidden.hide();

    reset_for_new_level();
}

init::CallbackOnStart player_init(&init);

} // namespace player
