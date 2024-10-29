#pragma once
#include "../../ecs/ecs.hpp"
#include <vector>

namespace touchscreen {

struct TouchObject;
std::vector<TouchObject*> touchables;

struct TouchObject: public ecs::Component {
	TouchObject(): ecs::Component() {
		touchables.push_back(this);
	}
	virtual ~TouchObject(){}
	virtual void handle_touch(glm::vec2 point) = 0;
};


}