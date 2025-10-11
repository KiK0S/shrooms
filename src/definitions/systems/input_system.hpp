#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include "../components/dynamic_object.hpp"
#include "../components/controllable_object.hpp"
#include "../../utils/exit.hpp"
#include "scene_system.hpp"
namespace input {

std::map<SDL_Scancode, bool> is_pressed;
std::unordered_map<SDL_FingerID, glm::vec2> active_touches;

bool get_button_state(SDL_Scancode key) {
	const Uint8* state = SDL_GetKeyboardState(nullptr);
	return state[key] || is_pressed[key];
}

glm::vec2 get_touch() {
	if (active_touches.empty()) {
		return glm::vec2(0.0f, 0.0f);
	}
	return active_touches.begin()->second;
}

struct Input: public dynamic::DynamicObject {
	Input(): dynamic::DynamicObject(-1) {}
	~Input(){
		Component::component_count--;
	}
	void update() {
		is_pressed.clear();
		while (true) {
			SDL_Event event;
			if (SDL_PollEvent(&event) == 0) {
					break;
			}
			fire_event(event);
			switch (event.type) {
			case SDL_KEYDOWN: {
				if (std::string(SDL_GetKeyName(event.key.keysym.sym)) == "Q") game_over::success();
				// if (std::string(SDL_GetKeyName(event.key.keysym.sym)) == "P") scene::toggle_pause();
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
				active_touches[event.tfinger.fingerId] = glm::vec2(x, y);
				break;
			}
			case SDL_FINGERUP: {
				active_touches.erase(event.tfinger.fingerId);
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


std::vector<glm::vec2> get_touches() {
	std::vector<glm::vec2> touches;
	touches.reserve(active_touches.size());
	for (const auto& [_, pos] : active_touches) {
		touches.push_back(pos);
	}
	return touches;
}

}
