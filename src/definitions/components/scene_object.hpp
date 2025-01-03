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
std::map<std::string, Scene*> scenes;

COMPONENT_MAP(SceneObject, scene_objects);

struct SceneObject : public ecs::Component {
	SceneObject(std::string scene_name) : ecs::Component(), scene_name(scene_name) {
		scene_objects[scene_name].push_back(this);
	}
	virtual ~SceneObject() {
		Component::component_count--;
	}
	Scene* get_scene() {
		return scenes[scene_name];
	}
	std::string scene_name;
	DETACH_MAP(SceneObject, scene_objects)
};

}
