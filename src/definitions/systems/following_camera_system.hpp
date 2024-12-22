#pragma once
#include "../definitions/components/dynamic_object.hpp"
#include "../definitions/components/transform_object.hpp"
#include "../ecs/ecs.hpp"
#include "glm/gtx/norm.hpp"

namespace camera {

struct PlayerFollowingSystem : public dynamic::DynamicObject {
    PlayerFollowingSystem(ecs::Entity* player, ecs::Entity* camera)
        : dynamic::DynamicObject(), 
          player_entity(player),
          camera_entity(camera) {}
    virtual PlayerFollowingSystem {
        Component::component_count--;
    }
    void update() override {
        transform::TransformObject* camera = camera_entity->get<transform::TransformObject>();
        transform::TransformObject* player = player_entity->get<transform::TransformObject>();

        glm::vec2 diff = player->get_pos() - camera->get_pos();
        float d = glm::length2(diff);
        
        if (d < 0.1)
            return;
            
        float v = 0.05;
        if (d > 0.4) {
            v *= 5;
        }
        camera->translate(diff * v);
    }

private:
    ecs::Entity* player_entity;
    ecs::Entity* camera_entity;
};

} // namespace camera