#pragma once
#include <string>
#include <vector>
#include <variant>
#include "ecs/component.hpp"

namespace debug {

class ObservableObject;
static std::vector<ObservableObject*> observables;

class ObservableObject : public ecs::Component {
public:
    ObservableObject() : ecs::Component() {
        observables.push_back(this);
    }
    virtual ~ObservableObject() {
        Component::component_count--;
    }

    virtual void register_imgui() = 0;
};
} // namespace debug 