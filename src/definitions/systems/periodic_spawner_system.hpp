#pragma once
#include "../components/periodic_spawner_object.hpp"
#include "../components/dynamic_object.hpp"
#include <chrono>

namespace periodic_spawn {

struct PeriodicSpawnerSystem : public dynamic::DynamicObject {
    PeriodicSpawnerSystem() : dynamic::DynamicObject() {}

    void update() override {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        if (last_update == std::nullopt) {
            last_update = now;
            return;
        }

        float dt = 1.0f * std::chrono::duration_cast<std::chrono::milliseconds>(now - *last_update).count() / 1000.0f;
        last_update = now;

        for (auto* spawner : periodic_spawners) {
            spawner->timer -= dt;
            // std::cerr << "spawner " << spawner->get_name() << " timer " << spawner->timer << std::endl;
            if (spawner->timer <= 0) {
                // Reset timer
                spawner->timer = spawner->period;
                // std::cerr << "spawner " << spawner->get_name() << " spawned" << std::endl;
                // Get entity position for spawn point
                auto entity = spawner->get_entity();
                auto geom_ptr = entity->get<geometry::GeometryObject>();
                if (!geom_ptr) continue;
                
                auto points = geom_ptr->get_pos();
                for (int i = 0; i < geom_ptr->get_size(); i += 3) {
                    std::vector<glm::vec2> new_points = spawning_system::spawn_points(points[i], points[i + 1], points[i + 2],
                                                                        spawner->rule.density);
                    for (auto point : new_points) {
                        point_objects.push_back(spawner->rule.spawn(point));
                    }
                }
            }
        }
    }

private:
    std::optional<std::chrono::time_point<std::chrono::system_clock>> last_update;
	std::vector<ecs::Entity*> point_objects;
};

} 