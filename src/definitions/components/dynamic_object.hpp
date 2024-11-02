#pragma once
#include "../../ecs/ecs.hpp"
#include "init_object.hpp"

namespace dynamic {

struct DynamicObject;

COMPONENT_VECTOR(DynamicObject, dynamics);

struct DynamicObject : public ecs::Component {
    DynamicObject(int priority = 0): priority(priority), ecs::Component() {
        dynamics.push_back(this);
    }
    virtual ~DynamicObject() {}
    virtual void update() = 0;
    int get_priority() {
        return priority;
    }
    int priority = 0;
	DETACH_VECTOR(DynamicObject, dynamics)
};

}