#pragma once

#include <vector>
#include "components/dynamic_object.hpp"
#include "components/controllable_object.hpp"
#include "components/minimap_object.hpp"
#include "utils/random.hpp"
#include "game_objects/player.hpp"
#include "systems/geometry_system.hpp"
#include "glm/glm/vec2.hpp"
#include "glm/gtx/norm.hpp"
#include "utils/arena.hpp"

namespace course {



struct Course {
	Course(int n) {
		positions.emplace_back(glm::vec2(0, 0));
		for (int i = 1; i < n; i++) {
			positions.emplace_back(glm::vec2(rnd::get_double(-10, 10), rnd::get_double(-10, 10)));
		}
		auto triangle = arena::create<geometry::Triangle>();
		add_minimap_entity(triangle, positions[0]);
		for (int i = 1; i < n; i++) {
			add_minimap_line(positions[i - 1], positions[i], i);			
			auto circle = arena::create<geometry::Circle>();
			add_minimap_entity(circle, positions[i]);
		}	
	}
	~Course() {}

	void add_minimap_entity(geometry::GeometryObject* g, glm::vec2 pos) {
		auto mini_e = arena::create<ecs::Entity>();
		auto transform = arena::create<transform::NoRotationTransform>();
		transform->scale(glm::vec2(0.1f, 0.1f));
		transform->translate(glm::vec2{pos.x * 0.1f, pos.y * 0.1f});
		auto program = arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program);
		auto matrix = arena::create<shaders::ModelMatrix>();
		mini_e->add(&color::pink);
		mini_e->add(transform);
		mini_e->add(matrix);
		mini_e->add(g);
		mini_e->add(program);
		mini_e->bind();
		auto minimaped = arena::create<minimap::MiniMapEntityPtr>(mini_e);
		auto e = std::make_unique<ecs::Entity>();
		e->add(minimaped);
		e->bind();
		es.push_back(std::move(e));
	}
	void add_minimap_line(glm::vec2 from, glm::vec2 to, int idx) {
		auto mini_e = arena::create<ecs::Entity>();
		auto g = arena::create<geometry::Curve>(std::string("leg-") + std::to_string(idx), std::vector<glm::vec2>{from, from, to, to});
		auto transform = arena::create<transform::NoRotationTransform>();
		transform->scale(glm::vec2(0.1f, 0.1f));
		auto matrix = arena::create<shaders::ModelMatrix>();
		auto program = arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program);
		mini_e->add(&color::pink);
		mini_e->add(transform);
		mini_e->add(g);
		mini_e->add(matrix);
		mini_e->add(program);
		mini_e->bind();
		auto minimaped = arena::create<minimap::MiniMapEntityPtr>(mini_e);
		auto e = std::make_unique<ecs::Entity>();
		e->add(minimaped);
		e->bind();
		es.push_back(std::move(e));
	}
	std::vector<glm::vec2> positions;
	std::vector<std::unique_ptr<ecs::Entity>> es;
	float control_radius = 0.05;
	uint32_t current_control = 0;

};
Course course(5);

struct CourseUpdate: public dynamic::DynamicObject {
	CourseUpdate(): dynamic::DynamicObject() {}
	void update() {
		if (course.current_control == course.positions.size())
			return;
		glm::vec2 diff = player::player.get<transform::TransformObject>()->get_pos() - course.positions[course.current_control];
		float d = glm::length2(diff);
		if (d > course.control_radius)
			return;

//  todo:
//		drawText(control_number);
		course.current_control++;
		std::cout << "found control\n";
	}
	void bind(ecs::Entity*) {}
};

struct CourseHint: public input::ControllableObject {
	CourseHint(): input::ControllableObject() {}
	void handle_user_action(SDL_Event e) {
		if (e.key.keysym.scancode == SDL_SCANCODE_E) {
			glm::vec2 diff = player::player.get<transform::TransformObject>()->get_pos() - course.positions[course.current_control];
			float d = glm::length2(diff);
			std::cout << d << '\n';
		}
	}
};

CourseUpdate course_update;
CourseHint course_hint;

}