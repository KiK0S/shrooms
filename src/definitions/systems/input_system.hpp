#pragma once
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <map>
#include "../components/dynamic_object.hpp"
#include "../components/controllable_object.hpp"
#include "../../utils/exit.hpp"

namespace input {

std::map<SDL_Scancode, bool> is_pressed;

bool get_button_state(SDL_Scancode key) {
	const Uint8* state = SDL_GetKeyboardState(nullptr);
	return state[key] || is_pressed[key];
}

glm::vec2 touch{0.0f, 0.0f};

glm::vec2 get_touch() {
	return touch;
}

struct Input: public dynamic::DynamicObject {
	Input(): dynamic::DynamicObject(-1) {}
	~Input(){}
	void update() {
		is_pressed.clear();
		while (true) {
			SDL_Event event;
			if (SDL_PollEvent(&event) == 0) {
					break;
			}
			switch (event.type) {
			case SDL_KEYDOWN: {
				fire_event(event);
				if (std::string(SDL_GetKeyName(event.key.keysym.sym)) == "Q") game_over::success();
				break;
			}
			case SDL_KEYUP: {
				break;
			}
			case SDL_MOUSEWHEEL: {
				int x, y;
				SDL_GetMouseState(&x, &y);
				// handle static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)
				break;
			}
			case SDL_MOUSEMOTION: {
				// handle static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP: {
				int x, y;
				SDL_GetMouseState(&x, &y);
				// handle static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)
				break;
			}
			case SDL_FINGERDOWN:
			case SDL_FINGERMOTION: {
				double x = event.tfinger.x;
				double y = event.tfinger.y;
				touch = glm::vec2(x, y);
				break;
			}
			case SDL_FINGERUP: {
				touch = {0.0f, 0.0f};
				break;
			}
			case SDL_QUIT:
				exit(0);
			}
		}
	}

	void fire_event(SDL_Event event) {
		for (auto* controllable : controllables) {
			controllable->handle_user_action(event);
		}
	}
};

}