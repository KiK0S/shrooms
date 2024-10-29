#pragma once
#include "glm/glm/vec2.hpp"
#include <string>
#include <memory>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <functional>

namespace serialization {

struct Serializable {
	virtual void serialize(std::stringstream& stream) = 0;
	virtual ~Serializable() {}
};

struct Deserializable {
	virtual void deserialize(std::string& obj) = 0;
	virtual ~Deserializable() {}
};

std::map<std::string, std::function<std::unique_ptr<Deserializable>(std::string&)>> factories;

#define DESERIALIZATION_REGISTER(className) \
	namespace { \
        struct Registrar { \
            Registrar() { \
                serialization::factories[#className] = [](std::string& data) { \
                    auto obj = std::make_unique<className>(); \
                    obj->deserialize(data); \
                    return obj; \
                }; \
            } \
        }; \
        static Registrar registrar; \
    }

void serialize(std::stringstream& stream, double d) {
	stream << d;
}
void serialize(std::stringstream& stream, size_t i) {
	stream << i;
}
void serialize(std::stringstream& stream, std::string s) {
	stream << s;
}
void serialize(std::stringstream& stream, glm::vec2 vec) {
	serialize(stream, vec.x);
	serialize(stream, ",");
	serialize(stream, vec.y);
}

std::string deserializeToken(std::string& s) {
	size_t pos = s.find("\n");
	std::string token = s.substr(0, pos);
	s.erase(0, pos);
	return token;
}
double deserializeFloat(std::string& s) {
	std::string token = deserializeToken(s);
	return stod(token);
}
int deserializeInt(std::string& s) {
	std::string token = deserializeToken(s);
	return stoi(token);
}
glm::vec2 deserializeVec2(std::string& s) {
	double x = deserializeFloat(s);
	double y = deserializeFloat(s);
	return glm::vec2(x, y);
}
std::unique_ptr<Deserializable> deserializeDeserializable(std::string& s) {
	std::string type = deserializeToken(s);
	return factories[type](s);
}
std::unique_ptr<Deserializable> deserialize(std::string& s) {
	return deserializeDeserializable(s);
}

std::string serialize(Serializable* s) {
	std::stringstream ss;
	s->serialize(ss);
	return ss.str();
}


};