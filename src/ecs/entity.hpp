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
		entity_count++;
	}
	virtual ~Entity() {
		entity_count--;
	}

	Entity& add(Component* comp) {
		comp->bind(this);
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
	bool is_pending_deletion() const { return to_delete; }
	void mark_deleted() { to_delete = true; }
	static inline std::vector<Entity*> entities;

	void detach(Component* comp) {
		// Remove from components list
		auto it = std::find(components.begin(), components.end(), comp);
		if (it != components.end()) {
			components.erase(it);
		}

		// Remove from pre-bind list if present
		it = std::find(pre_bind_components.begin(), pre_bind_components.end(), comp);
		if (it != pre_bind_components.end()) {
			pre_bind_components.erase(it);
		}

		// Unbind component from this entity
		comp->detach();
	}

	static size_t get_entity_count() { return entity_count; }

private:
	bool to_delete = false;
	std::vector<Component*> pre_bind_components;
	std::vector<Component*> components;
	static inline size_t entity_count = 0;
};

void reg_component(Entity* e, Component* c) {
	e->components.push_back(c);
}



// Template implementations
#include "entity.tpp"
}
