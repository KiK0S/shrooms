#pragma once
#include <vector>
#include <GL/gl.h>
#include "../../ecs/component.hpp"

namespace texture {

struct TexturedObject;

inline std::vector<TexturedObject*> textured;

struct TexturedObject : public ecs::Component {
	TexturedObject() : ecs::Component() {
		textured.push_back(this);
	}
	virtual ~TexturedObject() {}

	virtual GLuint get_texture() = 0;
};

}
