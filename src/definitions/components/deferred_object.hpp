#pragma once
#include "ecs/component.hpp"

namespace deferred {

struct DeferredObject;
COMPONENT_VECTOR(DeferredObject, deferred_objects);

struct DeferredObject : public ecs::Component {
    DeferredObject(std::function<void()> func, int delay_ms) : ecs::Component(), func(func), delay_ms(delay_ms) {
        start_time = std::chrono::steady_clock::now();
        deferred_objects.push_back(this);
    }
    virtual ~DeferredObject() {
        Component::component_count--;
    }
    DETACH_VECTOR(DeferredObject, deferred_objects);

    std::function<void()> func;
    int delay_ms;
    std::chrono::steady_clock::time_point start_time;
};
}