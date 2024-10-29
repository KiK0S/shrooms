#pragma once
#include <string>
#include "game_objects/sprite.hpp"
#include "utils/arena.hpp"
namespace tilemap {


std::unique_ptr<ecs::Entity> map_object(const std::string& name, double x, double y, double width, double height) {
	auto full_sprite = arena::create<sprite::Sprite>(name, glm::vec2{y - height / 2, x - width / 2}, glm::vec2{y + height / 2, x + width / 2}, 0);
	auto e = std::make_unique<ecs::Entity>();
	e->add(full_sprite);
	e->bind();
	return e;
}

struct TileMap {
	std::vector<std::unique_ptr<ecs::Entity>> tiles;
	TileMap(int n, int m) {
		double width = 1.0 / n;
		double height = 1.0 / m;

		for (int i = -n/2; i < n/2; i++) {
			for (int j = -m/2; j < m/2; j++) {
				tiles.emplace_back(map_object("tile_" + std::to_string(rnd::get_int(0, 2)), i * width, j * height, width, height));
			}
		}
	}
};
TileMap tilemap_data(100, 100);

ecs::Entity tilemap;
render::FramebufferTexture tilemap_texture;
shaders::ModelMatrix tilemap_model_matrix;
layers::ConstLayer tilemap_layer(0);
transform::NoRotationTransform tilemap_transform;
scene::SceneObject tilemap_scene("main");
shaders::ProgramArgumentObject tilemap_program(&shaders::raycast_program);



struct TilemapInit: public init::UnInitializedObject {
	TilemapInit(): init::UnInitializedObject(1) {}
	~TilemapInit() {}

	void init() {
		tilemap_texture.render_texture = render::create_render_target(GL_RGBA8, GL_RGBA);

		render::bind_render_target(&tilemap_texture.render_texture);


		glClearColor(1.0, 0.7, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (const auto& object : tilemap_data.tiles) {
			// shaders::run_with_program(object->get_entity()->get<shaders::ProgramArgumentObject>()->get_program(), [&](GLuint p) {
			// 	glUniform1i(glGetUniformLocation(p, "toFramebuffer"), false);
			// });
			render::display(object.get(), object->get<shaders::ProgramArgumentObject>()->get_program());
			// shaders::run_with_program(object->get_entity()->get<shaders::ProgramArgumentObject>()->get_program(), [&](GLuint p) {
			// 	glUniform1i(glGetUniformLocation(p, "toFramebuffer"), false);
			// });
		}
		render::bind_render_target(nullptr);

		tilemap_transform.scale(glm::vec2{10.0f, 10.0f});

		tilemap.add(&geometry::quad)
					.add(&tilemap_texture)
					.add(&tilemap_model_matrix)
					.add(&tilemap_model_matrix)
					.add(&tilemap_transform)
					.add(&tilemap_scene)
					.add(&tilemap_layer)
					.add(&tilemap_program)
					.bind();
	}
};

TilemapInit tilemap_init;

}