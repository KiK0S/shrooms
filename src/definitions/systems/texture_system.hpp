#pragma once
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glew.h>
#include <map>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../../utils/file_system.hpp"
#include "../components/stateful_object.hpp"

namespace texture {

GLuint create_texture(const int width, const int height, const void *data, GLuint color_mode = GL_RGBA, GLuint filter = GL_LINEAR) {
		GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, color_mode, width, height, 0, color_mode, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}
std::map<std::string, GLuint> texture_data;

GLuint get_texture_impl(std::string path) {
	if (texture_data.find(path) != texture_data.end()) {
		return texture_data[path];
	}
	GLuint res;
	if (path.empty()) {
		unsigned char data[4] = {255, 255, 255, 255};
		res = create_texture(1, 1, data);
	} else {
		int width, height, nrChannels;
		std::cout << path << '\n';
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data == nullptr) {
			std::cout << stbi_failure_reason() << std::endl;
			throw std::runtime_error(stbi_failure_reason());
		} else {
			res = create_texture(width, height, data);
			free(data);
		}
	}
	texture_data[path] = res;
	return res;
}

struct OneTextureObject: public TexturedObject {
	OneTextureObject(std::string name): TexturedObject(), name(name) {}
	GLuint get_texture() {
		return get_texture_impl(file::asset(name + ".png"));
	}
	std::string name;
};

struct IntTextureObject: public TexturedObject {
	IntTextureObject(GLuint texture): TexturedObject(), texture(texture) {}
	GLuint get_texture() {
		return texture;
	}
	GLuint texture;
};

struct NoTextureObject: public TexturedObject {
	NoTextureObject(): TexturedObject() {}
	GLuint get_texture() {
		return get_texture_impl("");
	}
};

struct AnimationTimer : public animation::AnimatedObject {
	AnimationTimer() : animation::AnimatedObject() {}

	void update(float dt) {
		time += dt;
	}
	float time = 0.0;
};

struct StateReset : public states::StringStateful {
	StateReset(std::function<void()>&& reset, std::string s): reset(std::move(reset)), states::StringStateful(s) {}
	void set_state(std::string state) {
		if (get_state() == state) return;
		states::StringStateful::set_state(state);
		reset();
	}
	std::function<void()> reset;
};

struct AnimatedTexture : public TexturedObject {
	AnimatedTexture(std::string name, std::map<std::string, std::vector<float>> data): name(name), TexturedObject(), animation(), state(std::bind(&AnimatedTexture::reset, this), data.begin()->first), durations(data) {}
	~AnimatedTexture() {}
	GLuint get_texture() {
		while (durations[state.get_state()][current_index] <= animation.time) {
			animation.time -= durations[state.get_state()][current_index];
			current_index += 1;
			if (current_index == durations[state.get_state()].size()) {
				current_index = 0;
			}
		}
		return get_texture_impl(file::asset(name + "_" + state.get_state() + "_" + std::to_string(current_index) + ".png"));
	}

	void reset() {
		animation.time = 0.0f;
		current_index = 0;
	}

	void bind(ecs::Entity* e) {
		TexturedObject::bind(e);
		animation.bind(e);
		state.bind(e);
	}
	AnimationTimer animation;
	std::map<std::string, std::vector<float>> durations;
	std::string name;
	StateReset state;
	size_t current_index = 0;
};


}