#pragma once
#include "../definitions/components/sprite_object.hpp"
#include "../definitions/components/collider_object.hpp"
#include "../definitions/systems/keyboard_movement_system.hpp"
#include "../definitions/systems/scene_system.hpp"
#include "../definitions/components/blinking_object.hpp"
#include <glm/glm/vec2.hpp>
#include "scoreboard.hpp"

namespace player {

keyboard_movement::KeyboardMovement player_control;
keyboard_movement::SpriteStateUpdate sprite_state_update(&player_control);
scene::SceneObject player_scene("main");
ecs::Entity player;
config::FloatParameter player_speed("Player speed", &player_control.velocity, 0.0f, 0.05f);
blinking::BlinkingObject player_blink;
hidden::HiddenObject player_hidden;

sprite::AnimatedSprite player_sprite("witch", {-1.0f, -1.0f}, {1.0f, 1.0f}, 2, {
    {"right", {0.2, 0.2}},
    {"left", {0.2, 0.2}}
});

ecs::Entity player_sprite_entity;
ecs::Entity player_trigger_entity;

collision::TriggerObject player_trigger("mushroom_catch_handler", 
    [](ecs::Entity* _player, collision::ColliderObject* mushroom) {
        LOG_IF(logger::enable_collision_system_logging, "Collected mushroom " << mushroom->get_entity()->get_checked<texture::OneTextureObject>()->name << "!");
        mushroom->get_entity()->mark_deleted();
        scoreboard::update_score(mushroom->get_entity()->get_checked<texture::OneTextureObject>()->name, scoreboard::eaten_mushrooms[mushroom->get_entity()->get_checked<texture::OneTextureObject>()->name] + 1);
    }
);

transform::NoRotationTransform player_transform;
transform::NoRotationTransform sprite_transform;
transform::NoRotationTransform trigger_transform;

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

    trigger_transform.scale_ = glm::vec2(0.5f, 0.2f);
    trigger_transform.translate(glm::vec2(-0.3f, 1.0f));
    player_trigger_entity.add(&trigger_transform)
                        .add(&player_trigger)
                        .add(&geometry::quad)
                        .add(&color::red)
                        .add(arena::create<layers::ConstLayer>(1))
                        .add(arena::create<shaders::ModelMatrix>())
                        .add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program))
                        .add(arena::create<scene::SceneObject>("main"))
                        .set_parent(&player)
                        .bind();
}

init::CallbackOnStart player_init(&init);

} // namespace player