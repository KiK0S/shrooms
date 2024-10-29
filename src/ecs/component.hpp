#pragma once
#include <string>
namespace ecs {
class Entity;  // Forward declaration
class Component;

void reg_component(Entity* e, Component* c);

class Component {
public:
    virtual ~Component() = default;
    virtual void bind(Entity* entity) {
        this->entity = entity;
		reg_component(entity, this);
    }
    virtual ecs::Entity* get_entity() {
        return entity;
    }
    Entity* entity = nullptr;
};
}
