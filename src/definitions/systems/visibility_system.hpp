#pragma once

#include "../components/dynamic_object.hpp"
#include "../components/shader_object.hpp"
#include "render_system.hpp"
#include "../../declarations/shader_system.hpp"
#include "../components/blocking_object.hpp"
#include <GL/glew.h>

namespace visibility_system {

struct ObstacleMap: public dynamic::DynamicObject, shaders::ShaderUniformsObject {
	ObstacleMap(): dynamic::DynamicObject(0), shaders::ShaderUniformsObject({&shaders::raycast_program, &shaders::bezier_raycast_program}) {}
	render_system::RenderTarget transparency_texture;

	bool have_init = false;
	void init() {
		transparency_texture = render_system::create_render_target(GL_R8, GL_RED);
		have_init = true;
	}
	void update() {
		if (!have_init)
			init();
		render_system::bind_render_target(&transparency_texture);
		glClearColor(0.0, 0.3, 0.3, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (const auto& obj : visibility::blocking_objects) {
			shaders::run_with_program(&shaders::static_object_program, [&](GLuint p) {
				glUniform1i(glGetUniformLocation(p, "toFramebuffer"), true);
				glUniform1i(glGetUniformLocation(p, "toView"), true);
			});
			render_system::display(obj->get_entity(), &shaders::static_object_program);
			shaders::run_with_program(&shaders::visibility_program, [&](GLuint p) {
				glUniform1i(glGetUniformLocation(p, "toFramebuffer"), false);
				glUniform1i(glGetUniformLocation(p, "toView"), false);
			});
		}
		render_system::bind_render_target(nullptr);
	}
	void reg_uniforms(GLuint program) {		
		auto transparancyTexture = glGetUniformLocation(program, "uTransparency");
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, transparency_texture.output_texture);
		glUniform1i(transparancyTexture, 1);
	}
};

} // namespace visibility_system
