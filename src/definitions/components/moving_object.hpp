#pragma once
#include <vector>
#include <GL/gl.h>
#include "../../ecs/component.hpp"
#include "transform_object.hpp"

namespace dynamic {

struct MovingObject : public DynamicObject {
    MovingObject(glm::vec2 translate) : DynamicObject(), translate(translate) {}
    virtual ~MovingObject() {}

    glm::vec2 translate;
    virtual void update() override {
        get_entity()->get<transform::TransformObject>()->translate(translate);
    }
};

}
