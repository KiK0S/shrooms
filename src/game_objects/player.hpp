#pragma once
#include "game_objects/sprite.hpp"
#include "../definitions/systems/input_system.hpp"
#include "../ecs/ecs.hpp"
#include "../definitions/systems/touchscreen_system.hpp"
#include "../definitions/components/dynamic_object.hpp"
#include "../definitions/components/stateful_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include "glm/glm/vec2.hpp"

namespace player {


struct KeyboardMovement : public dynamic::DynamicObject {
	KeyboardMovement(): dynamic::DynamicObject() {}
	~KeyboardMovement(){}
	double velocity = 0.02f;


	void update() {
		ecs::Entity* e = get_entity();
		glm::vec2 d{0.0f, 0.0f};

		if (input::get_button_state(SDL_SCANCODE_A)) {
			d.x -= 0.01;
		}

		if (input::get_button_state(SDL_SCANCODE_D)) {
			d.x += 0.01;
		}

		d += touchscreen::joystick_update.joystick_info;

		
		double len = glm::length(d);
		double k = len <= velocity ? 1.0 : velocity / len;
		d *= k;

		transform::TransformObject* t = e->get<transform::TransformObject>();
		t->translate(d);

		states::StatefulObject* a = e->get<states::StatefulObject>();

		if (len == 0) {
			a->set_state("left");
		} else if (d.x >= 0) {
			a->set_state("right");
		} else {
			a->set_state("left");
		}
	}
};

KeyboardMovement player_control;



scene::SceneObject player_scene("main");

ecs::Entity player;


sprite::AnimatedSprite player_sprite("witch", {-0.1f, -0.1f}, {0.1f, 0.1f}, 2, {
		{"right", {0.2, 0.2}},
		{"left", {0.2, 0.2}}
	});

void init() {
	std::cout << "init player\n";
	player.add(&player_sprite)
			.add(&player_control)
			.add(&player_scene)
			.bind();
}
init::CallbackOnStart player_init(&init);

}