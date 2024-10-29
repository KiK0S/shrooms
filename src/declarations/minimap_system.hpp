#pragma once

#include "components/controllable_object.hpp"
#include "components/minimap_object.hpp"
#include "components/layered_object.hpp"
#include "systems/geometry_system.hpp"
#include "systems/render_system.hpp"
#include "systems/easy_drawable_system.hpp"
#include "input_system.hpp"
#include "components/hidden_object.hpp"
#include <memory>

namespace minimap {

render::FramebufferTexture minimap_texture;

struct MapTextureInit: public init::UnInitializedObject {
	MapTextureInit(): init::UnInitializedObject(1) {}
	~MapTextureInit() {}

	void init() {
		std::cout << "create_map_texture\n";
		minimap_texture.render_texture = render::create_render_target(GL_RGBA8, GL_RGBA);
				std::cout << "create_render_target\n";

		render::bind_render_target(&minimap_texture.render_texture);
		std::cout << "bind_render_target\n";


		glClearColor(1.0, 0.7, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (const auto& object : minimap_objects) {
			shaders::run_with_program(object->get_entity()->get<shaders::ProgramArgumentObject>()->get_program(), [&](GLuint p) {
				glUniform1i(glGetUniformLocation(p, "toFramebuffer"), false);
			});
			render::display(object->get_entity(), object->get_entity()->get<shaders::ProgramArgumentObject>()->get_program());
			shaders::run_with_program(object->get_entity()->get<shaders::ProgramArgumentObject>()->get_program(), [&](GLuint p) {
				glUniform1i(glGetUniformLocation(p, "toFramebuffer"), false);
			});
		}
		render::bind_render_target(nullptr);
	}
};

transform::NoRotationTransform minimap_transform;
render::ModelMatrix minimap_model_matrix;
layers::ConstLayer minimap_layer{100};
MiniMapUniforms minimap_no_view_matrix;
MapTextureInit minimap_texture_init;

render::CombinedUniforms minimap_uniforms{{&minimap_model_matrix, &minimap_no_view_matrix}};

struct OpenMapAction: public input::ControllableObject {
	OpenMapAction(): input::ControllableObject() {

	}
	void handle_user_action(SDL_Event e) {
		if (e.type != SDL_KEYDOWN) return;
		if (e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			auto comp = get_entity()->get<render::HiddenObject>();
			comp->switch_state();
		}
	}


};

OpenMapAction open_on_space;
ecs::Entity minimap;
scene::SceneObject minimap_scene("main");
render::HiddenObject minimap_hidden;

void init() {
	minimap_transform.scale(glm::vec2(0.6f, 0.6f));
	auto minimap_program = arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program);
	minimap
				.add(&color::white)
				.add(&geometry::quad)
				.add(minimap_program)
				.add(&minimap_transform)
				.add(&minimap_no_view_matrix)
				.add(&minimap_model_matrix)
				.add(&minimap_texture)
				.add(&minimap_layer)
				 .add(&open_on_space)
				 .add(&minimap_scene)
				 .add(&minimap_hidden)
				 .bind();
}

init::CallbackOnStart minimap_init(&init);

}