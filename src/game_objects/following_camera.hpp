#pragma once
#include "components/gpu_program.hpp"
#include "components/transform_object.hpp"
#include "game_objects/player.hpp"
#include "game_objects/camera.hpp"
#include "glm/gtx/norm.hpp"

namespace camera {


struct PlayerFollowing : public dynamic::DynamicObject {
	PlayerFollowing(): dynamic::DynamicObject() {}
	~PlayerFollowing() {}
	void update() {
		ecs::Entity* e = get_entity();
		transform::TransformObject* camera = camera::camera.get<transform::TransformObject>();
		transform::TransformObject* player = player::player.get<transform::TransformObject>();

		glm::vec2 diff = player->get_pos() - camera->get_pos();
		float d = glm::length2(diff);
		if (d < 0.1)
			return;
		float v = 0.05;
		if (d > 0.4) {
			v *= 5;
		}
		camera->translate(diff * v);
	}
};

PlayerFollowing following_rule;

}