#pragma once

#include "../components/color_object.hpp"
#include <glm/glm.hpp>

namespace color {

struct OneColor: public ColoredObject {
	OneColor(glm::vec4 color, bool global = false): ColoredObject(), color(color) {
		global = true;
	}
	virtual ~OneColor() {
		Component::component_count--;
	}

	glm::vec4 get_color() {
		return color;
	}
	glm::vec4 color;
};

struct NoColorObject: public ColoredObject {
	NoColorObject(): ColoredObject() {
		global = true;
	}
	virtual ~NoColorObject() {
		Component::component_count--;
	}
	glm::vec4 get_color() {
		return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
	}
};


} // namespace color_system
