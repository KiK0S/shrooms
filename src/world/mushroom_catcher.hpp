#include "../definitions/systems/collision_system.hpp"

int score = 0;

// void mushroom_catch_handler(ecs::Entity* basket, collision::ColliderObject* collider) {
//     score += 1;
//     // disable collider: how??
// }

void mushroom_fall_handler(ecs::Entity* basket, collision::ColliderObject* collider) {
    std::cerr << "mushroom fall handler\n";
    score -= 1;
    // disable collider: how??
}

