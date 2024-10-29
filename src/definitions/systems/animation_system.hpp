#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <optional>	
#include "../components/dynamic_object.hpp"
#include "../components/animated_object.hpp"

namespace animation {

std::optional<std::chrono::time_point<std::chrono::system_clock>> m_time = {};

struct Animation : public dynamic::DynamicObject {
	Animation(): dynamic::DynamicObject() {}
	void update() {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		if (m_time == std::nullopt) {
			m_time = now;
			return;
		}
		float dt = 1.0f * std::chrono::duration_cast<std::chrono::milliseconds>(now - *m_time).count() / 1000.0f;
		m_time = now;

		for (auto* animated : animation::animateds) {
			animated->update(dt);
		}
	}
};

};