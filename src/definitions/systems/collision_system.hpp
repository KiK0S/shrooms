#pragma once

#include "../components/dynamic_object.hpp"
#include "geometry_system.hpp"
#include "../../ecs/ecs.hpp"
#include <map>
#include <vector>

namespace collision {

struct CollisionSystem: public dynamic::DynamicObject {
	CollisionSystem(): dynamic::DynamicObject() {}
	virtual ~CollisionSystem() {
		Component::component_count--;
	}
	void update() {
		for (auto [name, triggers] : trigger_objects) {
			for (auto trigger : triggers) {
				for (auto collider : collider_objects[name]) {
					auto collider_entity = collider->get_entity();
					auto trigger_entity = trigger->get_entity();
					if (collider_entity != nullptr && trigger_entity != nullptr) {
						// Use the get_bounding_box function for both trigger and collider
						geometry::BoundingBox trigger_bb = geometry::get_bounding_box(trigger_entity);
						geometry::BoundingBox collider_bb = geometry::get_bounding_box(collider_entity);
						
						// Check for intersection using the BoundingBox method
						if (trigger_bb.intersects(collider_bb)) {
							trigger->callback(trigger_entity, collider);
						}
					}
				}
			}
		}
	}
};

} // namespace collision_system
