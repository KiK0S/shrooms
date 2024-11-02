#pragma once
#include "../../ecs/ecs.hpp"
#include <vector>

namespace touchscreen {

struct TouchObject;
COMPONENT_VECTOR(TouchObject, touchables);

struct TouchObject: public ecs::Component {
	TouchObject(): ecs::Component() {
		touchables.push_back(this);
	}
	virtual ~TouchObject(){}
	virtual void handle_touch(glm::vec2 point) = 0;
	DETACH_VECTOR(TouchObject, touchables)
};


}