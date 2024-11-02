#pragma once
#include "glm/glm/vec4.hpp"
#include "../../ecs/ecs.hpp"
#include <vector>

namespace color {

struct ColoredObject;

COMPONENT_VECTOR(ColoredObject, coloreds);

struct ColoredObject: public ecs::Component {
	ColoredObject(): ecs::Component() {
		coloreds.push_back(this);
	}
	virtual ~ColoredObject(){}
	virtual glm::vec4 get_color() = 0;
	DETACH_VECTOR(ColoredObject, coloreds)
};



}
