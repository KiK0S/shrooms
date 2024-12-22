#pragma once
#include <vector>
#include <iostream>
#include "../../ecs/ecs.hpp"

namespace states {

struct StatefulObject;

COMPONENT_VECTOR(StatefulObject, statefuls);

struct StatefulObject: public ecs::Component {
	StatefulObject(): ecs::Component() {
		statefuls.push_back(this);
	}
	virtual ~StatefulObject() {
		Component::component_count--;
	}

	virtual std::string get_state() = 0;
	virtual void set_state(std::string) = 0;
	DETACH_VECTOR(StatefulObject, statefuls)
};

struct StringStateful: public StatefulObject {
	StringStateful(std::string state): StatefulObject(), state(state) {
	}
	virtual ~StringStateful() {
		Component::component_count--;
	}

	std::string state;
	virtual std::string get_state() {
		return state;
	}
	virtual void set_state(std::string new_state) {
		state = new_state;
	}
};

}