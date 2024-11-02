#pragma once

#include "../../ecs/ecs.hpp"

namespace cleanup {

class CleanupSystem : public dynamic::DynamicObject {
public:
    CleanupSystem() : dynamic::DynamicObject(10000) {} // High priority to run last

    void update() override {
        // Remove from global entity list
        auto& entities = ecs::Entity::entities;
        entities.erase(
            std::remove_if(entities.begin(), entities.end(),
                [](ecs::Entity* e) {
                    if (e->is_pending_deletion()) {
                        // Clean up components
                        cleanup_entity(e);
                        return true;
                    }
                    return false;
                }
            ),
            entities.end()
        );
    }

private:
    static void cleanup_entity(ecs::Entity* entity) {
        for (auto& pair : ecs::component_registries) {
            if (ecs::container_is_vector[pair.first]) {
                auto vec = static_cast<std::vector<ecs::Component*>*>(pair.second);
                vec->erase(
                    std::remove_if(vec->begin(), vec->end(),
                        [entity](ecs::Component* c) { return c->get_entity() == entity; }),
                    vec->end()
                );
            } else {
                auto map = static_cast<std::map<std::string, std::vector<ecs::Component*>>*>(pair.second);
                for (auto& pair : *map) {
                    pair.second.erase(
                        std::remove_if(pair.second.begin(), pair.second.end(),
                            [entity](ecs::Component* c) { return c->get_entity() == entity; }),
                        pair.second.end()
                    );
                }
            }
        }
    }
};

} // namespace cleanup