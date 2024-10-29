#pragma once

#include "glm/glm/vec2.hpp"
#include <string>

struct PointObject : public serialization::Serializable, serialization::Deserializable {
	PointObject(){}
	PointObject(std::string name, glm::vec2 pos): pos(pos), name(name) {}
	std::string name;
	glm::vec2 pos;
	void serialize(std::stringstream& ss) {
		serialization::serialize(ss, "point");
		serialization::serialize(ss, name);
		serialization::serialize(ss, pos);
	}
	void deserialize(std::string& s) {
		name = serialization::deserializeToken(s);
		pos = serialization::deserializeVec2(s);
	}
};
DESERIALIZATION_REGISTER(PointObject)