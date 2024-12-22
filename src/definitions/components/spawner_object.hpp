#pragma once
#include <vector>
#include "../../ecs/ecs.hpp"


namespace spawn {

struct SpawnerObject;

COMPONENT_VECTOR(SpawnerObject, spawners);

struct SpawningRule {
	double density;
	std::function<ecs::Entity*(glm::vec2)> spawn;
};

struct SpawnerObject: public ecs::Component {
	SpawnerObject(): ecs::Component() {
		spawners.push_back(this);
	}
	virtual ~SpawnerObject() {
		Component::component_count--;
	}
	DETACH_VECTOR(SpawnerObject, spawners)
	virtual std::vector<SpawningRule> get_rules() = 0;
};

struct SpawnerRuleContainer: public SpawnerObject {
	SpawnerRuleContainer(std::vector<SpawningRule> rules): SpawnerObject(), rules(rules) {}
	virtual ~SpawnerRuleContainer() {
		Component::component_count--;
	}
	std::vector<SpawningRule> get_rules() {
		return rules;
	}
	std::vector<SpawningRule> rules;
};
}