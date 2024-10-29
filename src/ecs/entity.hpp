#pragma once
#include "component.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
namespace ecs {

class Entity {
public:
	Entity() {
		entities.push_back(this);
	}
	virtual ~Entity() {}

	Entity& add(Component* comp) {
		pre_bind_components.push_back(comp);
		return *this;
	}
	Entity& bind() {
		for (Component* c : pre_bind_components) {
			c->bind(this);
		}
		return *this;
	}

	template <typename T>
	std::vector<T*> get_all() const;

	template <typename T>
	T* get() const;

	template <typename T>
	T* get_checked() const;
	friend void reg_component(Entity* e, Component* c);

private:
	std::vector<Component*> pre_bind_components;
	std::vector<Component*> components;

	static inline std::vector<Entity*> entities;
};

void reg_component(Entity* e, Component* c) {
	e->components.push_back(c);
}



// Template implementations
#include "entity.tpp"
}
