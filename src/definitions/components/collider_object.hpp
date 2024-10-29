#pragma once
#include "../../ecs/component.hpp"
#include <string>
#include <map>
#include <vector>
#include <functional>

namespace collision {
struct ColliderObject;
struct TriggerObject;

inline std::map<std::string, std::vector<ColliderObject*>> collider_objects;
inline std::map<std::string, std::vector<TriggerObject*>> trigger_objects;

struct ColliderObject : public ecs::Component {
    ColliderObject(std::string type) : ecs::Component(), type(type) {
        collider_objects[type].push_back(this);
    }
    virtual ~ColliderObject() {}

    std::string type;
};

typedef std::function<void(ecs::Entity*, ColliderObject*)> TriggerCallback;

struct TriggerObject : public ecs::Component {
    TriggerObject(std::string type, TriggerCallback callback) : ecs::Component(), type(type), callback(callback) {
        trigger_objects[type].push_back(this);
    }
    virtual ~TriggerObject() {}

    std::string type;
    TriggerCallback callback;
};

}
