#pragma once
#include "../definitions/components/geometry_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include <memory>
#include <vector>

namespace geometry {


struct BezierCurve : public geometry::GeometryObject, shaders::ShaderUniformsObject {
	BezierCurve(std::string name, std::array<glm::vec2, 4> points): name(name), points(points), geometry::GeometryObject(), shaders::ShaderUniformsObject({}) {}
	
	int n = 30;
	std::vector<glm::vec2> get_pos() {
		std::vector<glm::vec2> result;
		for (int i = 0; i + 1 < n; ++i) {
			float t = i / float(n- 1);
			result.push_back(glm::vec2{t * 2 - 1.0f, -1.0f});   // x position (mapped to t)
			result.push_back(glm::vec2{(t + 1.0f / float(n-1)) * 2 - 1.0f, -1.0f});   // x position (mapped to t)
			result.push_back(glm::vec2{t * 2 - 1.0f, 1.0f});   // x position (mapped to t)

			result.push_back(glm::vec2{(t + 1.0f / float(n-1)) * 2 - 1.0f, -1.0f});   // x position (mapped to t)
			result.push_back(glm::vec2{(t + 1.0f / float(n-1)) * 2 - 1.0f, 1.0f});   // x position (mapped to t)
			result.push_back(glm::vec2{t * 2 - 1.0f, 1.0f});   // x position (mapped to t)
			
		}
		return result;
	}

	std::vector<glm::vec2> get_uv() {
		std::vector<glm::vec2> result;
		for (int i = 0; i + 1 < n; ++i) {
			for (int j = 0; j < 6; j++)
				result.push_back(glm::vec2{0.0f, 0.0f});
		}
		return result;
	}
	int get_size() {
		// (n - 1) * 6
		return (n - 1) * 6;
	}
	std::string name;
	std::string get_name() const {
		return name;
	}
	void reg_uniforms(GLuint program) {
		float* data = reinterpret_cast<GLfloat *>(points.data());
		glUniform2fv(glGetUniformLocation(program, "controlPoints"), 4, reinterpret_cast<GLfloat *>(points.data()));
	}
	std::array<glm::vec2, 4> points;
};


struct Curve : public geometry::GeometryObject {
	Curve(std::string name, std::vector<glm::vec2> points): name(name), geometry::GeometryObject() {
		glm::vec2 prev = points[0];
	
		for (int i = 1; i + 2 < points.size(); i += 2) {
			glm::vec2 next = i + 4 < points.size() ? (points[i + 1] + points[i + 2]) / 2.0f : points[i + 2];
			add_curve_segment(std::array{
													prev, 
													points[i],
													points[i + 1],
													next}, i);			
			prev = next;
		}
	}

	void add_curve_segment(std::array<glm::vec2, 4> points, int idx) {
		std::vector<glm::vec2> xs(n);
		std::vector<glm::vec2> normals(n);

		glm::vec2 P0 = points[0];
		glm::vec2 P1 = points[1];
		glm::vec2 P2 = points[2];
		glm::vec2 P3 = points[3];
		for (int i = 0; i < n; i++) {
			float t = i * 1.0 / n;
			float u = 1.0 - t;
			glm::vec2 bezier_pos = u*u*u * P0 +
							3.0f * u*u * t * P1 +
							3.0f * u * t*t * P2 +
							t*t*t * P3;
			glm::vec2 tangent = -3.0f * u*u * P0 +
								3.0f * (u*u - 2.0f * u * t) * P1 +
								3.0f * (2.0f * t * u - t*t) * P2 +
								3.0f * t*t * P3;
			tangent = glm::normalize(tangent);
			glm::vec2 normal = glm::normalize(glm::vec2(-tangent.y, tangent.x));
			xs[i] = bezier_pos;
			normals[i] = normal;
		}
		float width = 0.02f;
		for (int top_down = 0; top_down < 2; top_down++) {
			for (int i = 1; i < n; i++) {
				positions.push_back(xs[i - 1] + width * normals[i - 1]);
				positions.push_back(xs[i - 1] - width * normals[i - 1]);
				positions.push_back(xs[i] - width * normals[i]);

				positions.push_back(xs[i] - width * normals[i]);
				positions.push_back(xs[i] + width * normals[i]);
				positions.push_back(xs[i - 1] + width * normals[i - 1]);
			}
		}
	}
	
	int n = 30;
	std::vector<glm::vec2> get_pos() {
		return positions;
	}

	std::vector<glm::vec2> get_uv() {
		std::vector<glm::vec2> result;
		for (int i = 0; i < positions.size(); ++i) {
			result.push_back(glm::vec2{0.0f, 0.0f});
		}
		return result;
	}
	int get_size() {
		return positions.size();
	}
	std::string name;
	std::string get_name() const {
		return name;
	}
	std::vector<glm::vec2> positions;

};

}