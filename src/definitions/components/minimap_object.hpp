#pragma once

#include <vector>
#include <memory>
#include "../../ecs/component.hpp"

namespace minimap {
	

struct MiniMapObject;

COMPONENT_VECTOR(MiniMapObject, minimap_objects);

struct MiniMapObject : public ecs::Component {
	MiniMapObject() : ecs::Component() {
		minimap_objects.push_back(this);
	}
	~MiniMapObject() {
		Component::component_count--;
	}
	virtual ecs::Entity* get_entity() override {
		return ecs::Component::get_entity();
	}
	DETACH_VECTOR(MiniMapObject, minimap_objects)
};

struct MiniMapEntity : public MiniMapObject {
	MiniMapEntity(std::unique_ptr<ecs::Entity> e) : e(std::move(e)), MiniMapObject() {}
	~MiniMapEntity() {
		Component::component_count--;
	}
	std::unique_ptr<ecs::Entity> e;
	virtual ecs::Entity* get_entity() override {
		return e.get();
	}
};

struct MiniMapEntityPtr : public MiniMapObject {
	MiniMapEntityPtr(ecs::Entity* e) : e(e), MiniMapObject() {}
	~MiniMapEntityPtr() {
		Component::component_count--;
	}
	ecs::Entity* e;
	virtual ecs::Entity* get_entity() override {
		return e;
	}
};

}
