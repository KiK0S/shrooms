#pragma once
#include "../../ecs/ecs.hpp"
#include "spawner_object.hpp"
#include <chrono>

namespace periodic_spawn {

struct PeriodicSpawnerObject;
COMPONENT_VECTOR(PeriodicSpawnerObject, periodic_spawners);

struct PeriodicSpawnerObject : public ecs::Component {
    PeriodicSpawnerObject(float period, spawn::SpawningRule rule)
        : period(period), rule(rule), timer(period), ecs::Component() {
        periodic_spawners.push_back(this);
    }
    
    virtual ~PeriodicSpawnerObject() {
        Component::component_count--;
    }
    DETACH_VECTOR(PeriodicSpawnerObject, periodic_spawners)

    float period;  // Time between spawns in seconds
    float timer;   // Current time until next spawn
    spawn::SpawningRule rule;
};

} 