#pragma once
#include "../definitions/components/geometry_object.hpp"

namespace geometry {
struct Triangle : public geometry::GeometryObject {

	Triangle(): geometry::GeometryObject() {}
	~Triangle() {}
	virtual std::vector<glm::vec2> get_pos() {
		return {
			glm::vec2{0.0f, 0.0f}, // Vertex 1: top left
			glm::vec2{1.0f, 0.0f}, // Vertex 2: top right
			glm::vec2{0.0f, 1.0f}, // Vertex 3: bottom left
		};
	}
	virtual std::vector<glm::vec2> get_uv() {
		return {
			glm::vec2{0.0f,  0.0f}, // Vertex 1: top left
			glm::vec2{1.0f,  0.0f}, // Vertex 2: top right
			glm::vec2{0.0f, 1.0f}, // Vertex 3: bottom left
		};
	}
	int get_size() {
		return 3;
	}
	virtual std::string get_name() const {
		return "triangle";
	}
};

Triangle triangle;

}