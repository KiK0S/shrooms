#pragma once
#include "../../ecs/component.hpp"

namespace hidden {
struct HiddenObject : public ecs::Component {
	HiddenObject() : ecs::Component() {
		Component::component_count--;
	}
	void hide() {
		visible = false;
	}
	void show() {
		visible = true;
	}
	bool is_visible() {
		return visible;
	}
	void switch_state() {
		if (is_visible()) {
			hide();
		} else {
			show();
		}
	}

	bool visible = false;
};
}
