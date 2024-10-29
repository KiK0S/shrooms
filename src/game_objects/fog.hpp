#pragma once
#include "game_objects/sprite.hpp"
#include "components/gpu_program.hpp"
#include "components/textured_object.hpp"
#include "components/init_object.hpp"
#include <GL/gl.h>

namespace fog {

struct Background: public shaders::ShaderUniformsObject {
	Background(): shaders::ShaderUniformsObject({&shaders::raycast_program, &shaders::bezier_raycast_program}) {}

	void reg_uniforms(GLuint program) {
		ecs::Entity* e = get_entity();
		texture::TexturedObject* t = e->get<texture::TexturedObject>();
		auto backgroundTexture = glGetUniformLocation(program, "uBackground");
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, t->get_texture());
		glUniform1i(backgroundTexture, 2);
	}
};

Background background_uniform;
sprite::SpriteCustomProgram background_sprite("cloud", {-1.0f, -1.0f}, {1.0f, 1.0f}, 0, &shaders::static_object_program);
ecs::Entity background;

void init() {
	background.add(&background_sprite)
						.add(&background_uniform).bind();
}

init::CallbackOnStart background_init(&init);
}