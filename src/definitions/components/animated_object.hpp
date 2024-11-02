#pragma once
#include <vector>
#include <string>
#include "../../ecs/ecs.hpp"

namespace animation {

struct AnimatedObject;
COMPONENT_VECTOR(AnimatedObject, animateds);

struct AnimatedObject : public ecs::Component {
	AnimatedObject() : ecs::Component() {
		animateds.push_back(this);
	}
	virtual ~AnimatedObject() {}
	virtual void update(float dt) = 0;
	DETACH_VECTOR(AnimatedObject, animateds);
};

}
