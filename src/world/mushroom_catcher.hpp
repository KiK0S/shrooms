#include "../definitions/systems/collision_system.hpp"
#include "../definitions/components/collider_object.hpp"
#include "../utils/callback_registry.hpp"

void mushroom_fall_handler(ecs::Entity* basket, collision::ColliderObject* collider) {
    LOG_IF(logger::enable_collision_system_logging, "Fallen mushroom!");
    collider->get_entity()->mark_deleted();
    scoreboard::update_score(0);
}

static collision::TriggerCallbackRegistry::Registrar mushroom_fall_registrar("mushroom_fall_handler", mushroom_fall_handler);
