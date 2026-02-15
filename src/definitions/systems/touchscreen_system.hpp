#pragma once

#include "../components/dynamic_object.hpp"
#include "../../declarations/input_system.hpp"
#include "../components/touch_object.hpp"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <cmath>
#include "../../utils/arena.hpp"
#include "../../geometry/circle.hpp"
#include "../../geometry/quad.hpp"
#include "../../declarations/uniform_system.hpp"
#include "../../declarations/shader_system.hpp"
#include "../../declarations/texture_system.hpp"
#include "../../declarations/color_system.hpp"
#include "../../declarations/scene_system.hpp"
#include "../components/transform_object.hpp"
#include "../components/layered_object.hpp"
#include "../components/shader_object.hpp"
#include "../components/textured_object.hpp"
#include "../components/scene_object.hpp"
#include "../components/hidden_object.hpp"

namespace touchscreen {

struct TouchKeyboardButton;


std::unique_ptr<ecs::Entity> joystick;
std::unique_ptr<ecs::Entity> joystick_inner;
shaders::MiniMapUniforms joystick_uniform;
std::unique_ptr<ecs::Entity> fire_button;
std::unique_ptr<TouchKeyboardButton> fire_button_touch;
const glm::vec2 fire_button_center(0.75f, -0.6f);
const glm::vec2 fire_button_half_extent(0.13f, 0.13f);
hidden::HiddenObject joystick_hidden;
hidden::HiddenObject joystick_inner_hidden;
hidden::HiddenObject fire_button_hidden;



struct JoystickUpdate: public TouchObject {
	JoystickUpdate(): TouchObject() {}

	glm::vec2 joystick_info{0.0f, 0.0f};

	void update_joystick() {
		auto joystick_transform = joystick_inner->get<transform::TransformObject>();
		joystick_transform->translate(-joystick_transform->get_pos() + glm::vec2(-0.6f, -0.6f) + joystick_info * 0.1f);
	}

	void handle_touch(glm::vec2 point) {
		if (!is_within_bounds(point)) {
			return;
		}
		double dx = (point.x + 0.6) / 0.25;
		double dy = (point.y + 0.6) / 0.25;
		joystick_info = {dx, dy};
		if (dx < -1) joystick_info /= -dx;
		if (dx > 1) joystick_info /= dx;
		if (dy < -1) joystick_info /= -dy;
		if (dy > 1) joystick_info /= dy;
		update_joystick();
		joystick_info.y *= -1;
	}

	bool is_within_bounds(glm::vec2 point) const {
		double dx = point.x + 0.6;
		double dy = point.y + 0.6;
		return std::abs(dx) <= 0.5 && std::abs(dy) <= 0.5;
	}

	void reset() {
		if (joystick_info == glm::vec2(0.0f, 0.0f)) {
			return;
		}
		joystick_info = {0.0f, 0.0f};
		update_joystick();
	}
};
JoystickUpdate joystick_update;

void init_joystick() {
	joystick = std::make_unique<ecs::Entity>();
	auto joystick_drawable = arena::create<render::SolidDrawable>(&geometry::circle, &shaders::static_object_program);
	joystick_drawable->get_transform()->scale(glm::vec2(0.25f, 0.25f));
	joystick_drawable->get_transform()->translate(glm::vec2(-0.6f, -0.6f));
	joystick_drawable->color = &color::lighter_grey;
	joystick_drawable->uniforms.add(&joystick_uniform);
	auto joystick_scene = arena::create<scene::SceneObject>("main");
	joystick->add(joystick_drawable);
	joystick->add(joystick_scene);
	joystick_hidden.hide();
	joystick->add(&joystick_hidden);
	joystick->bind();

	joystick_inner = std::make_unique<ecs::Entity>();
	auto joystick_inner_drawable = arena::create<render::SolidDrawable>(&geometry::circle, &shaders::static_object_program);
	joystick_inner_drawable->get_transform()->scale(glm::vec2(0.2f, 0.2f));
	joystick_inner_drawable->get_transform()->translate(glm::vec2(-0.6f, -0.6f));
	joystick_inner_drawable->color = &color::light_grey;
	joystick_inner_drawable->uniforms.add(&joystick_uniform);
	auto joystick_inner_scene = arena::create<scene::SceneObject>("main");
	joystick_inner->add(joystick_inner_drawable);
	joystick_inner->add(joystick_inner_scene);
	joystick_inner_hidden.hide();
	joystick_inner->add(&joystick_inner_hidden);
	joystick_inner->bind();
}

void init_fire_button() {
	glm::vec2 top_left = fire_button_center - fire_button_half_extent;
	glm::vec2 bottom_right = fire_button_center + fire_button_half_extent;
	fire_button_touch = std::make_unique<TouchKeyboardButton>(SDL_SCANCODE_W, top_left, bottom_right);

	fire_button = std::make_unique<ecs::Entity>();
	fire_button->add(&geometry::quad);
	fire_button->add(arena::create<layers::ConstLayer>(6));
	fire_button->add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program));

	auto transform = arena::create<transform::NoRotationTransform>();
	transform->scale(fire_button_half_extent);
	transform->translate(fire_button_center);
	fire_button->add(transform);
	fire_button->add(arena::create<shaders::ModelMatrix>());
	fire_button->add(arena::create<texture::OneTextureObject>("explosion"));
	fire_button->add(&color::white);
	fire_button->add(arena::create<scene::SceneObject>("main"));
	fire_button_hidden.hide();
	fire_button->add(&fire_button_hidden);
	fire_button->bind();
}


struct TouchKeyboardButton: public TouchObject {
	TouchKeyboardButton(SDL_Scancode key, glm::vec2 top_left, glm::vec2 bottom_right) : TouchObject(), key(key), top_left(top_left), bottom_right(bottom_right) {}
	void handle_touch(glm::vec2 pos) {
		if (pos.x < top_left.x || pos.x > bottom_right.x || pos.y < top_left.y || pos.y > bottom_right.y) {
			last_pushed = std::chrono::time_point<std::chrono::system_clock>::min();
			return;
		}
		std::chrono::time_point<std::chrono::system_clock> cur_time = std::chrono::system_clock::now();
		
		input::is_pressed[key] = true;
		if ((cur_time - last_pushed).count() < 300000) return;
		SDL_Event event;
		event.key.keysym.scancode = key;
		input::input.fire_event(event);
		last_pushed = cur_time;
	}
	std::chrono::time_point<std::chrono::system_clock> last_pushed = std::chrono::system_clock::now();
	SDL_Scancode key;
	glm::vec2 top_left;
	glm::vec2 bottom_right;
};

struct TouchSystem: public dynamic::DynamicObject {
	TouchSystem(): dynamic::DynamicObject(-1) {}

	bool inited = false;
	bool ui_active = false;

	void init() {
		touchscreen::init_joystick();
		touchscreen::init_fire_button();
		inited = true;
	}

	void update() {
		if (!inited && SDL_GetNumTouchDevices() > 0) init();
		if (!inited) return;
		auto raw_touches = input::get_touches();
		if (raw_touches.empty()) {
			joystick_update.reset();
			return;
		}
		if (!ui_active) {
			ui_active = true;
			joystick_hidden.show();
			joystick_inner_hidden.show();
			fire_button_hidden.show();
		}
        std::vector<glm::vec2> normalized;
        normalized.reserve(raw_touches.size());
        bool joystick_consumed = false;
        for (const auto& touch : raw_touches) {
            glm::vec2 point(touch.x * 2.0f - 1.0f, 1.0f - touch.y * 2.0f);
            normalized.push_back(point);
            if (!joystick_consumed && joystick_update.is_within_bounds(point)) {
                joystick_update.handle_touch(point);
                joystick_consumed = true;
            }
		}
		if (!joystick_consumed) {
			joystick_update.reset();
		}
		for (auto touch_object : touchables) {
			if (touch_object == &joystick_update) {
				continue;
			}
			for (const auto& point : normalized) {
				touch_object->handle_touch(point);
			}
		}
	}
};
}
