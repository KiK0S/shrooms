#pragma once
#include <vector>
#include "glm/glm/vec2.hpp"
#include "../../ecs/component.hpp"
#include <memory>

namespace visibility {
struct BlockingObject;
inline std::vector<BlockingObject*> blocking_objects;

struct BlockingObject : public ecs::Component {
	BlockingObject(): ecs::Component() {
		blocking_objects.push_back(this);
	}
	virtual ecs::Entity* get_entity() override {
		return ecs::Component::get_entity();
	}
	virtual ~BlockingObject() {}
};


struct BlockingEntity: public BlockingObject {
	BlockingEntity(std::unique_ptr<ecs::Entity> e): e(std::move(e)), BlockingObject() {}
	~BlockingEntity() {}
	std::unique_ptr<ecs::Entity> e;
	virtual ecs::Entity* get_entity() override {
		return e.get();
	}
};

struct BlockingEntityPtr: public BlockingObject {
	BlockingEntityPtr(ecs::Entity* e): e(e), BlockingObject() {}
	~BlockingEntityPtr() {}
	ecs::Entity* e;
	virtual ecs::Entity* get_entity() override {
		return e;
	}
};
}
