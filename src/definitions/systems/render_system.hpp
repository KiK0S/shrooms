#pragma once

#include "../components/textured_object.hpp"
#include "../components/dynamic_object.hpp"
#include "../components/text_object.hpp"
#include "gpu_program_system.hpp"
#include "../components/hidden_object.hpp"
#include "../../declarations/color_system.hpp"
#include "texture_system.hpp"
#include "../../declarations/texture_system.hpp"

#include <SDL2/SDL.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include "glm/glm/vec4.hpp"
#include "glm/glm/vec2.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace render_system {
SDL_Window* _window;

struct RenderTarget {
	GLuint frame_buffer;
	GLuint output_texture;
};


struct FramebufferTexture : public texture::TexturedObject {
	FramebufferTexture(): texture::TexturedObject() {}
	~FramebufferTexture() {
		Component::component_count--;
	}

	RenderTarget render_texture;
	GLuint get_texture() {	
		return render_texture.output_texture;
	};
};

struct FrameStartSystem: public dynamic::DynamicObject {
	FrameStartSystem(): dynamic::DynamicObject(-1) {}
	void update() {
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Enable depth testing (objects can appear behind/infront of eachother)
		// glEnable(GL_DEPTH_TEST);
		// glDepthFunc(GL_LEQUAL);

		// Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
};

struct FrameEndSystem: public dynamic::DynamicObject {
	FrameEndSystem(): dynamic::DynamicObject(1000) {}
	void update() {
		SDL_GL_SwapWindow(_window);
	}
};


RenderTarget create_render_target(GLenum internalFormat, GLenum format) {
	GLuint framebuffer = 0;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLuint output_texture;
	glGenTextures(1, &output_texture);

	glBindTexture(GL_TEXTURE_2D, output_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 960, 960, 0, format, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture, 0);

	GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, draw_buffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("broken framebuffer");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
	return RenderTarget{framebuffer, output_texture};
}


void bind_render_target(RenderTarget* target) {
	if (target == nullptr) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, target->frame_buffer);
}



std::map<std::string, GLuint> vao_data;

void add_to_frame(geometry::GeometryObject* object) {
	if (vao_data.find(object->get_name()) != vao_data.end()) {
		return;
	}
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	auto positionBuffer = buffers[0];
	auto uvBuffer = buffers[1];
	

	GLuint vaos[1];
	glGenVertexArrays(1, vaos);

	auto vao = vaos[0];

	glBindVertexArray(vao);
	// Position data
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 2 * object->get_size(),
							object->get_pos().data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

	// Texture data
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 2 * object->get_size(), object->get_uv().data(),
								GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
	vao_data[object->get_name()] = vao;
}



void display(ecs::Entity* entity, shaders::Program* program_ptr) {
	shaders::run_with_program(program_ptr, [&](GLuint program){
		glBindVertexArray(vao_data[entity->get<geometry::GeometryObject>()->get_name()]);
		auto hidden_comp = entity->get<hidden::HiddenObject>();
		if (hidden_comp != nullptr && !hidden_comp->is_visible()) {
			return;
		}
		auto textureLocation = glGetUniformLocation(program, "uTexture");
		glActiveTexture(GL_TEXTURE0);
		auto texture_comp = entity->get<texture::TexturedObject>();
		if (texture_comp == nullptr) {
			texture_comp = &texture::no_texture;
		}
		glBindTexture(GL_TEXTURE_2D, texture_comp->get_texture());
		glUniform1i(textureLocation, 0);

		auto colorLocation = glGetUniformLocation(program, "uColor");

		auto color_comp = entity->get<color::ColoredObject>();
		if (color_comp != nullptr) {
			glUniform4fv(colorLocation, 1, glm::value_ptr(color_comp->get_color()));
		} else {
			glUniform4fv(colorLocation, 1, glm::value_ptr(color::white.get_color()));
		}
		program_ptr->reg_uniforms(program);
		auto uniforms_comps = entity->get_all<shaders::ShaderUniformsObject>();
		for (auto uniforms_comp : uniforms_comps) {
			uniforms_comp->reg_uniforms(program);
		}

        bool has_text = entity->get<text::TextGeometry>() != nullptr;
        GLboolean cull_enabled = glIsEnabled(GL_CULL_FACE);
        if (has_text && cull_enabled) {
            glDisable(GL_CULL_FACE);
        }

        glDrawArrays(GL_TRIANGLES, 0, entity->get<geometry::GeometryObject>()->get_size());

        if (has_text && cull_enabled) {
            glEnable(GL_CULL_FACE);
        }
    });

}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

void init(SDL_Window *window) {
	_window = window;
	std::cout << "SDL_GL_CreateContext" << std::endl;

	auto *context = SDL_GL_CreateContext(window);
	std::cout << "glewInit" << std::endl;

	const unsigned char* version = glGetString(GL_VERSION);
	std::cout << "opengl version " << version << '\n';
	
	// todo: fix quad, polygon
	glEnable(GL_CULL_FACE);

	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus) << std::endl;
		throw std::runtime_error("Failed to initialize GLEW");

	}
	
	shaders::init();

	// During init, enable debug output
	#ifndef __EMSCRIPTEN__
	glEnable              ( GL_DEBUG_OUTPUT );
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback( MessageCallback, 0 );
	#endif
}

} // namespace render_system
