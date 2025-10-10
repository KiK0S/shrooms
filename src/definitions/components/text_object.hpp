#pragma once
#include "../../ecs/component.hpp"
#include "geometry_object.hpp"


namespace text {
	struct TextObject;
	COMPONENT_VECTOR(TextObject, texts);


	struct TextObject: public ecs::Component {
		TextObject(std::string text): text(text), ecs::Component() {
			texts.push_back(this);
		}
		virtual ~TextObject() {
			Component::component_count--;
		}
		virtual std::string get_text() {
			return text;
		}
		std::string text;
		DETACH_VECTOR(TextObject, texts)
	};


	struct TextGeometry : public geometry::GeometryObject {
		TextGeometry(std::string name): name(name), geometry::GeometryObject() {}
		virtual ~TextGeometry() {
			Component::component_count--;
		}
		std::vector<glm::vec2> get_pos() {
			return pos;
		}
		int get_size() {
			return pos.size();
		}
		std::vector<glm::vec2> get_uv() {
			return uv;
		}
		std::string get_name() const {
			return name;
		}
		std::string name;
		std::vector<glm::vec2> pos;
		std::vector<glm::vec2> uv;
		float logical_width = 0.0f;
		float logical_height = 0.0f;
	};

 
}
