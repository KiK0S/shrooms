#pragma once
#include <vector>
#include <string>
#include "glm/glm/vec2.hpp"
#include "glm/glm/mat4x4.hpp"
#include "../../ecs/component.hpp"

namespace geometry {

struct GeometryObject;

COMPONENT_VECTOR(GeometryObject, geometry_objects);

struct GeometryObject : public ecs::Component {
	GeometryObject() : ecs::Component() {
		geometry_objects.push_back(this);
		global = true;
	}
	virtual ~GeometryObject() {
		Component::component_count--;
	}

	virtual std::vector<glm::vec2> get_pos() = 0;
	virtual int get_size() = 0;
	virtual std::vector<glm::vec2> get_uv() = 0;
	virtual std::string get_name() const = 0;
	DETACH_VECTOR(GeometryObject, geometry_objects)
};

}
