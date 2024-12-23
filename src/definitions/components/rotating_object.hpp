#pragma once
#include <vector>
#include <GL/gl.h>
#include "../../ecs/component.hpp"
#include "transform_object.hpp"

namespace dynamic {

struct RotatingObject : public DynamicObject {
    RotatingObject(float angle) : DynamicObject(), angle(angle) {}
    virtual ~RotatingObject() {
        Component::component_count--;
    }

    float angle;
    virtual void update() override {
        if (scene::is_current_scene_paused()) {
            return;
        }
        get_entity()->get<transform::TransformObject>()->rotate(angle);
    }
};

}
