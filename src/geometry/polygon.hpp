#pragma once
#include "../definitions/components/geometry_object.hpp"
#include "delaunay-cpp/delaunay.hpp"
#include "../definitions/systems/spawning_system.hpp"
#include <memory>
#include "glm/glm/vec4.hpp"


namespace geometry {

struct Polygon : public geometry::GeometryObject {
	Polygon(std::string name, std::vector<glm::vec2> points): geometry::GeometryObject(), name(name) {
		std::vector<delaunay::Point<float>> data(points.size());
		for (int i = 0; i < points.size(); i++)
			data[i] = delaunay::Point<float>(points[i].x, points[i].y);
		auto res = delaunay::triangulate(data);
		this->points.resize(res.triangles.size() * 3);
		this->uv.assign(res.triangles.size() * 3, glm::vec2{0.0f, 0.0f});
		for (int i = 0; i < res.triangles.size(); i++) {
			this->points[3 * i + 0] = glm::vec2{res.triangles[i].p0.x, res.triangles[i].p0.y};
			this->points[3 * i + 1] = glm::vec2{res.triangles[i].p1.x, res.triangles[i].p1.y};
			this->points[3 * i + 2] = glm::vec2{res.triangles[i].p2.x, res.triangles[i].p2.y};
		}
	}
	~Polygon() {
		Component::component_count--;
	}
	Polygon(std::string name, std::vector<glm::vec2> points, std::vector<spawn::SpawningRule> rules): Polygon(name, points) {}
	std::vector<glm::vec2> get_pos() {
		return points;
	}
	std::vector<glm::vec2> get_uv() {
		return uv;
	}
	int get_size() {
		return points.size();
	}
	std::string get_name() const {
		return name;
	}


	std::string name;
	std::vector<glm::vec2> points;
	std::vector<glm::vec2> uv;
};

}