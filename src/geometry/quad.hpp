#pragma once
#include "../definitions/components/animated_object.hpp"
#include "../definitions/systems/render_system.hpp"
#include "../utils/file_system.hpp"
#include <functional>
#include <string>

namespace geometry {
	struct Quad : public geometry::GeometryObject {

		Quad(std::string name, std::vector<glm::vec2> pos): geometry::GeometryObject(), name(name), pos({
			pos[0], pos[1], pos[2], pos[1], pos[3], pos[2] 
		}) {
			uv = {
				glm::vec2{0.0f,  1.0f}, // Vertex 1: top left
				glm::vec2{1.0f,  1.0f}, // Vertex 2: top right
				glm::vec2{0.0f, 0.0f}, // Vertex 3: bottom left

				glm::vec2{1.0f,  1.0f}, // Vertex 4: top right
				glm::vec2{1.0f, 0.0f},  // Vertex 6: bottom right
				glm::vec2{0.0f, 0.0f} // Vertex 5: bottom left
			};
		}

		Quad(): Quad("quad", {
				glm::vec2{-1.0f, -1.0f}, // Vertex 1: top left
				glm::vec2{1.0f, -1.0f}, // Vertex 2: top right
				glm::vec2{-1.0f, 1.0f}, // Vertex 3: bottom left
				glm::vec2{1.0f, 1.0f},  // Vertex 6: bottom right
			}) {}
		~Quad() {
			Component::component_count--;
		}
		virtual std::vector<glm::vec2> get_pos() {
			return pos;
		}
		virtual std::vector<glm::vec2> get_uv() {
			return uv; 
		}
		virtual int get_size() {
			return pos.size();
		}
		virtual std::string get_name() const {
			return name;
		}
		std::string name;
		std::vector<glm::vec2> pos, uv;
	};

	Quad quad;

}