#pragma once

#include "../components/dynamic_object.hpp"
#include "../../ecs/ecs.hpp"
#include "../components/shader_object.hpp"
#include "../../declarations/logger_system.hpp"
#include <string>
#include <map>
#include <iostream>

namespace scene {

std::map<std::string, Scene*> scenes;

struct Scene: public dynamic::DynamicObject {
	Scene(std::string name): name(name), dynamic::DynamicObject(10) {
		scene::scenes[name] = this;
		is_active = false;
		is_paused = false;
	}
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
	bool is_paused;
	void toggle_pause() {
		is_paused = !is_paused;
		LOG_IF(logger::enable_scene_system_logging, "Scene " << name << (is_paused ? " paused" : " resumed"));
	}
	void set_pause(bool paused) {
		is_paused = paused;
		LOG_IF(logger::enable_scene_system_logging, "Scene " << name << (is_paused ? " paused" : " resumed"));
	}
	bool is_paused_state() const {
		return is_paused;
	}
};

inline bool is_current_scene_paused() {
	for (const auto& [name, scene] : scenes) {
		if (scene->is_active) {
			return scene->is_paused;
		}
	}
	return false;
}

inline void toggle_pause() {
	for (const auto& [name, scene] : scenes) {
		if (scene->is_active) {
			scene->toggle_pause();
		}
	}
}


struct cmp {
	bool operator()(SceneObject* a, SceneObject* b) const {
		return a->get_entity()->get<layers::LayeredObject>()->get_layer() < b->get_entity()->get<layers::LayeredObject>()->get_layer();
	}
};

} // namespace scene_system
