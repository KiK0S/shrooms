#pragma once
#include "../../ecs/component.hpp"
#include <string>
#include <map>
#include <vector>
#include <functional>

namespace collision {
struct ColliderObject;
struct TriggerObject;

COMPONENT_MAP(ColliderObject, collider_objects);
COMPONENT_MAP(TriggerObject, trigger_objects);

struct ColliderObject : public ecs::Component {
    ColliderObject(std::string type) : ecs::Component(), type(type) {
        collider_objects[type].push_back(this);
    }
    virtual ~ColliderObject() {
        Component::component_count--;
    }

    std::string type;
	DETACH_MAP(ColliderObject, collider_objects)
};

typedef std::function<void(ecs::Entity*, ColliderObject*)> TriggerCallback;

struct TriggerObject : public ecs::Component {
    TriggerObject(std::string type, TriggerCallback callback) : ecs::Component(), type(type), callback(callback) {
        trigger_objects[type].push_back(this);
    }
    virtual ~TriggerObject() {
        Component::component_count--;
    }

    std::string type;
    TriggerCallback callback;
	DETACH_MAP(TriggerObject, trigger_objects)
};

}
