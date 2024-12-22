#pragma once

#include "../components/dynamic_object.hpp"
#include "../../ecs/ecs.hpp"
#include "../components/shader_object.hpp"
#include <string>
#include <map>
#include <iostream>

namespace scene {

std::map<std::string, Scene*> scenes;

struct Scene: public dynamic::DynamicObject {
	Scene(std::string name): name(name), dynamic::DynamicObject(10) {
		scene::scenes[name] = this;
		is_active = false;
	virtual ~Scene() {
		Component::component_count--;
	}
	std::string get_name() {
		return name;
	}
	void update() {
		if (!is_active)
			return;
		
		LOG_IF(logger::enable_scene_system_logging, "scene " << get_name() << " has " << scene_objects[get_name()].size() << " objects");
		for (auto* object : scene_objects[get_name()]) {
			render_system::display(object->get_entity(), object->get_entity()->get<shaders::ProgramArgumentObject>()->get_program());
		}
	}
	void activate() {
		for (auto it = scenes.begin(); it != scenes.end(); ++it) {
			it->second->is_active = false;
		}
		is_active = true;
	}
	bool is_active;
	std::string name;
};


struct cmp {
	bool operator()(SceneObject* a, SceneObject* b) const {
		return a->get_entity()->get<layers::LayeredObject>()->get_layer() < b->get_entity()->get<layers::LayeredObject>()->get_layer();
	}
};

} // namespace scene_system
