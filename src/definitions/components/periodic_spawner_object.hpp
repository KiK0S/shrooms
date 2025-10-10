#pragma once
#include "../../ecs/ecs.hpp"
#include "spawner_object.hpp"
#include <chrono>
#include <string>
#include <utility>

namespace periodic_spawn {

struct PeriodicSpawnerObject;
COMPONENT_VECTOR(PeriodicSpawnerObject, periodic_spawners);

struct PeriodicSpawnerObject : public ecs::Component {
    PeriodicSpawnerObject(float period, spawn::SpawningRule rule, std::string spawn_type = "")
        : period(period), rule(rule), timer(period), spawn_type(std::move(spawn_type)), ecs::Component() {
        periodic_spawners.push_back(this);
    }
    
    virtual ~PeriodicSpawnerObject() {
        Component::component_count--;
    }
    DETACH_VECTOR(PeriodicSpawnerObject, periodic_spawners)

    void configure(float new_period, double density, int max_spawns) {
        period = new_period;
        timer = period;
        rule.density = density;
        max_spawn_count = max_spawns;
        spawned_count = 0;
        enabled = true;
    }

    bool can_spawn_more() const {
        return max_spawn_count < 0 || spawned_count < max_spawn_count;
    }

    bool is_depleted() const {
        return max_spawn_count >= 0 && spawned_count >= max_spawn_count;
    }

    float period;  // Time between spawns in seconds
    float timer;   // Current time until next spawn
    spawn::SpawningRule rule;
    bool enabled = true;
    int max_spawn_count = -1;
    int spawned_count = 0;
    std::string spawn_type;
};

} 