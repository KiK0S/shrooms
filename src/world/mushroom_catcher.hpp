#include "../definitions/systems/collision_system.hpp"
#include "../definitions/components/collider_object.hpp"
#include "../utils/callback_registry.hpp"
#include "lives.hpp"
#include "../definitions/components/blinking_object.hpp"
#include "player.hpp"

void mushroom_fall_handler(ecs::Entity* basket, collision::ColliderObject* collider) {
    LOG_IF(logger::enable_collision_system_logging, "Fallen mushroom!");
    collider->get_entity()->mark_deleted();
    lives::remove_life();
    if (!player::player_blink.is_blinking()) {
        player::player_blink.activate(0.2f, 3); // Blink 3 times with 0.2s period
    }
    if (lives::current_lives == 0) {
        for (auto& [name, count] : scoreboard::eaten_mushrooms) {
            scoreboard::update_score(name, 0);
        }
        lives::reset_lives();
    }
}

static collision::TriggerCallbackRegistry::Registrar mushroom_fall_registrar("mushroom_fall_handler", mushroom_fall_handler);
