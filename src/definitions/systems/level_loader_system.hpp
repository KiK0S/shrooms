#pragma once

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include "../../ecs/ecs.hpp"
#include "component_loader_system.hpp"
namespace level_loader {

void load(std::string filename) {
	std::ifstream in(filename);
	auto entities = parse(in);
	for (auto e : entities) {
		auto scene = arena::create<scene::SceneObject>("main");
		e->add(scene);
		scene->bind(e);
	}
}


}