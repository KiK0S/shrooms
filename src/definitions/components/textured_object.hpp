#pragma once
#include <vector>
#include <GL/gl.h>
#include "../../ecs/component.hpp"

namespace texture {

struct TexturedObject;

COMPONENT_VECTOR(TexturedObject, textured);

struct TexturedObject : public ecs::Component {
	TexturedObject() : ecs::Component() {
		textured.push_back(this);
	}
	virtual ~TexturedObject() {
		Component::component_count--;
	}

	virtual GLuint get_texture() = 0;
	DETACH_VECTOR(TexturedObject, textured)
};

}
