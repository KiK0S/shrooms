#pragma once
#include "../definitions/components/sprite_object.hpp"
#include "../definitions/components/collider_object.hpp"
#include "../definitions/systems/keyboard_movement_system.hpp"
#include "../definitions/systems/scene_system.hpp"
#include <glm/glm/vec2.hpp>
#include "scoreboard.hpp"

namespace player {

keyboard_movement::KeyboardMovement player_control;
scene::SceneObject player_scene("main");
ecs::Entity player;
config::FloatParameter player_speed("Player speed", &player_control.velocity, 0.0f, 0.05f);

const float player_level = -0.6f;
sprite::AnimatedSprite player_sprite("witch", {-0.1f, -0.1f + player_level}, {0.1f, 0.1f + player_level}, 2, {
    {"right", {0.2, 0.2}},
    {"left", {0.2, 0.2}}
});

collision::TriggerObject mushroom_trigger("mushroom_catch_handler", 
    [](ecs::Entity* player, collision::ColliderObject* mushroom) {
        // Handle mushroom collection
        LOG_IF(logger::enable_collision_system_logging, "Collected mushroom " << mushroom->get_entity()->get_checked<texture::OneTextureObject>()->name << "!");
        mushroom->get_entity()->mark_deleted();
        scoreboard::update_score(mushroom->get_entity()->get_checked<texture::OneTextureObject>()->name, scoreboard::eaten_mushrooms[mushroom->get_entity()->get_checked<texture::OneTextureObject>()->name] + 1);
    }
);

void init() {
    std::cout << "init player\n";
    player.add(&player_sprite)
          .add(&player_control)
          .add(&player_scene)
          .add(&player_speed)
          .add(&mushroom_trigger)
          .bind();
}

init::CallbackOnStart player_init(&init);

} // namespace playerx