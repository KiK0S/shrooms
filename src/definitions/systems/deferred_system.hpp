#pragma once
#include "../components/deferred_object.hpp"

namespace deferred {

struct DeferredSystem : public dynamic::DynamicObject {
    DeferredSystem() : dynamic::DynamicObject(50) {}

    void update() override {
        for (auto deferred : deferred_objects) {
            if (deferred->start_time + std::chrono::milliseconds(deferred->delay_ms) < std::chrono::steady_clock::now()) {
                deferred->func();
                deferred->get_entity()->mark_deleted();
            }
        }
    }
};

void fire_deferred(std::function<void()>&& func, float delay) {
    auto e = arena::create<ecs::Entity>();
    e->add(arena::create<DeferredObject>(std::move(func), delay));
}
}