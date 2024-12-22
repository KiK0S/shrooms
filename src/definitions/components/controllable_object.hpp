#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "../../ecs/ecs.hpp"

namespace input {
struct ControllableObject;
COMPONENT_VECTOR(ControllableObject, controllables);

struct ControllableObject : public 	ecs::Component {
	ControllableObject(): ecs::Component() {
		controllables.push_back(this);
	}
	virtual ~ControllableObject() {
		Component::component_count--;
	}
	virtual void handle_user_action(SDL_Event event) = 0;
	DETACH_VECTOR(ControllableObject, controllables)
};

}