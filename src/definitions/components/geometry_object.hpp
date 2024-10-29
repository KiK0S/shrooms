#pragma once
#include <vector>
#include <string>
#include "glm/glm/vec2.hpp"
#include "glm/glm/mat4x4.hpp"
#include "../../ecs/component.hpp"

namespace geometry {

struct GeometryObject;

inline std::vector<GeometryObject*> geometry_objects;

struct GeometryObject : public ecs::Component {
	GeometryObject() : ecs::Component() {
		geometry_objects.push_back(this);
	}
	virtual ~GeometryObject() {}

	virtual std::vector<glm::vec2> get_pos() = 0;
	virtual int get_size() = 0;
	virtual std::vector<glm::vec2> get_uv() = 0;
	virtual std::string get_name() const = 0;
};

}
