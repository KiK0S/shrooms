
#pragma once
#include <vector>
#include <GL/gl.h>
#include "../../ecs/ecs.hpp"

namespace layers {

struct LayeredObject;

COMPONENT_VECTOR(LayeredObject, layereds);

struct LayeredObject: public ecs::Component {
	LayeredObject(): ecs::Component() {
		layereds.push_back(this);
	}
	virtual ~LayeredObject() {}

	virtual int get_layer() const = 0;
	DETACH_VECTOR(LayeredObject, layereds)
};

struct ConstLayer: public LayeredObject {
	ConstLayer(int layer): LayeredObject(), layer(layer) {}
	int get_layer() const {
		return layer;
	};
	int layer;
};


}