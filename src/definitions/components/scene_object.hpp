#pragma once
#include <string>
#include <map>
#include <vector>
#include "dynamic_object.hpp"
#include "init_object.hpp"
#include "../../ecs/component.hpp"
#include "layered_object.hpp"

namespace scene {

struct Scene;
struct SceneObject;

COMPONENT_MAP(SceneObject, scene_objects);

struct SceneObject : public ecs::Component {
	SceneObject(std::string scene_name) : ecs::Component() {
		scene_objects[scene_name].push_back(this);
	}
	virtual ~SceneObject() {
		Component::component_count--;
	}
	DETACH_MAP(SceneObject, scene_objects)
};

}
