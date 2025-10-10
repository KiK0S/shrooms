#pragma once

#include "../../ecs/ecs.hpp"
#include "geometry_system.hpp"
#include "../components/moving_object.hpp"
#include "../components/collider_object.hpp"
#include "../components/periodic_spawner_object.hpp"
#include "../components/rotating_object.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <glm/glm.hpp>

namespace levels {
void register_spawner(periodic_spawn::PeriodicSpawnerObject* spawner);
void on_mushroom_spawned(const std::string& type, ecs::Entity* entity);
}

namespace level_loader {

ecs::Entity* parse_entity(std::istream& in, shaders::Program* program = &shaders::static_object_program);
std::vector<ecs::Entity*> parse(std::istream& in);


std::string parse_entity_description(std::istream& in) {
	std::string name;
	in >> name;
	std::string res = name;
	std::string token;
	while (in >> token) {
		res = res + ' ' + token;
		if (token == name) {
			break;
		}
	}
	return res;
}
geometry::GeometryObject* parse_geometry(std::istream& in, std::string name) {
	std::string type;
	in >> type;

	int n;
	if (type == "quad") n = 4;
	else in >> n;
	std::vector<glm::vec2> points(n);
	for (int i = 0; i < n; i++) {
		in >> points[i].x >> points[i].y;
	}
	if (type == "polygon") {
		return arena::create<geometry::Polygon>(name, points);
	} else if (type == "quad") {
		return arena::create<geometry::Quad>(name, points);
	} else {
		return arena::create<geometry::Curve>(name, points);
	}
}

float parse_float(std::istream& in) {
	std::string token;
	in >> token;
	if (token == "random") {
		float a, b;
		in >> a >> b;
		return rnd::get_double(a, b);
	}
	return std::stof(token);
}

dynamic::MovingObject* parse_moving(std::istream& in) {
	glm::vec2 point;
	in >> point.x >> point.y;
	return arena::create<dynamic::MovingObject>(point);
}

dynamic::RotatingObject* parse_rotating(std::istream& in) {
	float angle = parse_float(in);
	return arena::create<dynamic::RotatingObject>(angle);
}

texture::TexturedObject* parse_texture(std::istream& in) {
	std::string filepath;
	in >> filepath;
	return arena::create<texture::OneTextureObject>(filepath);
}

layers::LayeredObject* parse_layer(std::istream& in) {
	int layer_num;
	in >> layer_num;
	return arena::create<layers::ConstLayer>(layer_num);
}


collision::ColliderObject* parse_collider(std::istream& in) {
	std::string handler_name;
	in >> handler_name;
	return arena::create<collision::ColliderObject>(handler_name);
}

collision::TriggerObject* parse_trigger(std::istream& in) {
	std::string handler_name;
	in >> handler_name;
	auto callback = collision::TriggerCallbackRegistry::get_callback(handler_name);
	if (!callback) {
		std::cerr << "Warning: No trigger callback registered for name: " << handler_name << std::endl;
		callback = [](ecs::Entity*, collision::ColliderObject*) {}; // Empty callback as fallback
	}
	return arena::create<collision::TriggerObject>(handler_name, callback);
}

color::ColoredObject* parse_color(std::istream& in) {
	int r;
	int g;
	int b;
	int a;
	in >> r >> g >> b >> a;
	return arena::create<color::OneColor>(glm::vec4{r / 255.0, g / 255.0, b / 255.0, a / 255.0});
}

spawn::SpawnerObject* parse_spawner(std::istream& in) {
	double density;
	in >> density;
	std::cerr << density << std::endl;
	auto desc = parse_entity_description(in);
	std::cerr << density << std::endl << desc << std::endl;
	return arena::create<spawn::SpawnerRuleContainer>(std::vector{
															spawn::SpawningRule{
																0.75,
																[=](glm::vec2 pos) {
																	std::cerr << desc << std::endl;
																	std::istringstream new_e_desc(desc);
																	auto new_e = parse_entity(new_e_desc);
																	new_e->get<transform::TransformObject>()->translate(pos);
																	new_e->get<transform::TransformObject>()->scale({0.1f, 0.1f});
																	new_e->add(&geometry::quad);

																	auto scene = arena::create<scene::SceneObject>("main");
																	new_e->add(scene);
																	new_e->bind();
																	return new_e;
																}
															}
														});
}

minimap::MiniMapObject* parse_minimap(std::istream& in) {
	auto e = parse_entity(in, &shaders::static_object_program);
	e->add(&texture::no_texture);
	e->add(&shaders::no_uniforms);
	e->get<transform::TransformObject>()->scale({0.1f, 0.1f});
	return arena::create<minimap::MiniMapEntityPtr>(e);
}

periodic_spawn::PeriodicSpawnerObject* parse_periodic_spawner(std::istream& in) {
        float period;
        double density;
        in >> period >> density;

        auto desc = parse_entity_description(in);
        auto texture_name = extract_texture_name(desc);
        auto spawner = arena::create<periodic_spawn::PeriodicSpawnerObject>(
                period,
                spawn::SpawningRule{
                        density,
                        [=](glm::vec2 pos) {
                                std::istringstream new_e_desc(desc);
                                auto new_e = parse_entity(new_e_desc);
                                if (!new_e) return static_cast<ecs::Entity*>(nullptr);
                                new_e->get<transform::TransformObject>()->scale({0.1f, 0.1f});
                                new_e->get<transform::TransformObject>()->translate(pos);
                                new_e->add(&geometry::quad);

                                auto scene = arena::create<scene::SceneObject>("main");
                                new_e->add(scene);
                                new_e->bind();
                                levels::on_mushroom_spawned(texture_name, new_e);
                                return new_e;
                        }
                },
                texture_name
        );
        spawner->enabled = false;
        spawner->max_spawn_count = 0;
        spawner->spawned_count = 0;
        levels::register_spawner(spawner);
        return spawner;
}

ecs::Entity* parse_entity(std::istream& in, shaders::Program* program) {
	std::string name;

	if (!(in >> name)) return nullptr;
	ecs::Entity* e = arena::create<ecs::Entity>();
	std::string comp;
	// std::cerr << "parsing entity " << name << std::endl;
	while (in >> comp) {
		if (comp == name) break;
		// std::cerr << "parsed component " << comp << std::endl;
		if (comp == "geometry") e->add(parse_geometry(in, name));
		if (comp == "texture") e->add(parse_texture(in));
		if (comp == "color") e->add(parse_color(in));
		if (comp == "layer") e->add(parse_layer(in));
		if (comp == "minimap") e->add(parse_minimap(in));
		if (comp == "spawner") e->add(parse_spawner(in));
		if (comp == "moving") e->add(parse_moving(in));
		if (comp == "rotating") e->add(parse_rotating(in));
		if (comp == "collider") e->add(parse_collider(in));
		if (comp == "trigger") e->add(parse_trigger(in));
		if (comp == "periodic_spawner") e->add(parse_periodic_spawner(in));
	}

	// std::cerr << "parsed entity " << name << std::endl;
	auto model_matrix = arena::create<shaders::ModelMatrix>();
	e->add(model_matrix);
	auto translate = arena::create<transform::LocalRotationTransform>();
	e->add(translate);
	e->add(arena::create<shaders::ProgramArgumentObject>(program));
	e->bind();
	return e;
}

std::vector<ecs::Entity*> parse(std::istream& in) {
	std::vector<ecs::Entity*> res;
	ecs::Entity* e;
	while ((e = parse_entity(in)) != nullptr) {		
		res.push_back(e);
	}
	return res;
}

}
std::string extract_texture_name(const std::string& desc) {
        std::istringstream stream(desc);
        std::string token;
        if (!(stream >> token)) {
                return "";
        }
        while (stream >> token) {
                if (token == "texture") {
                        std::string texture_name;
                        stream >> texture_name;
                        return texture_name;
                }
        }
        return "";
}

