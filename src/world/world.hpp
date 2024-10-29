#pragma once
#include <memory>
#include <vector>

#include "game_objects/sprite.hpp"
#include "systems/render_system.hpp"
#include "components/blocking_object.hpp"

#include "utils/random.hpp"
#include "utils/arena.hpp"
#include "game_objects/tilemap.hpp"

namespace map
{

uint8_t color[4] =	{56, 91, 94, 255};

std::unique_ptr<ecs::Entity> map_object(const std::string& name, double x, double y, double width, double height) {
	auto full_sprite = arena::create<sprite::Sprite>(name, glm::vec2{y - height / 2, x - width / 2}, glm::vec2{y + height / 2, x + width / 2}, 3);
	
	auto minimap_sprite = arena::create<sprite::SpriteCustomProgram>(name, glm::vec2{(y) / 10.0 - 0.05, (x) / 10.0 - 0.05}, glm::vec2{(y) / 10.0 + 0.05, (x) / 10.0 + 0.05}, 3, &shaders::static_object_program);
	auto mini_entity = arena::create<ecs::Entity>();
	mini_entity->add(minimap_sprite);
	mini_entity->bind();
	auto minimap_object = arena::create<minimap::MiniMapEntityPtr>(mini_entity);
	auto circle = arena::create<geometry::Circle>();
	auto circle_drawable = arena::create<render::SolidDrawable>(circle);
	circle_drawable->transform.scale(glm::vec2{0.2f, 0.2f});
	circle_drawable->transform.translate(glm::vec2(y, x));
	
	auto blocking_entity = arena::create<ecs::Entity>();
	blocking_entity->add(circle_drawable);
	blocking_entity->bind();
	auto blocking_object = arena::create<visibility::BlockingEntityPtr>(blocking_entity);

	auto main_scene = arena::create<scene::SceneObject>("main");
	auto e = std::make_unique<ecs::Entity>();
	e->add(full_sprite);
	e->add(minimap_object);
	e->add(blocking_object);
	e->add(main_scene);
	e->bind();

	return e;
}

struct World {

	std::vector<std::unique_ptr<ecs::Entity>> objects;
	int n, m;

	World(int n, int m): n(n), m(m) {
		int treesCnt = 50;
		int rockCnt = 20;

		for (int i = 0; i < treesCnt; i++) {
			objects.emplace_back(map_object("tree", rnd::get_double(-10, 10), rnd::get_double(-10, 10), 0.4, 0.4));
		}
		for (int i = 0; i < rockCnt; i++) {
			objects.emplace_back(map_object("rock", rnd::get_double(-10, 10), rnd::get_double(-10, 10), 0.3, 0.3));
		}
	}
};

World world(100, 100);

} // namespace map
