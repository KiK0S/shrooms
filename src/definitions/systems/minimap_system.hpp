#pragma once

#include "../components/init_object.hpp"
#include "../render/render_target.hpp"
#include "../shaders/shaders.hpp"
#include "../components/program_argument_object.hpp"
#include <GL/glew.h>
#include <iostream>

namespace minimap_system {

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

} // namespace minimap_system
