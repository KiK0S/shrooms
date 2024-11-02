#pragma once
#include <vector>
#include "transform_object.hpp"
#include "dynamic_object.hpp"
#include "gpu_program.hpp"
#include "../../ecs/component.hpp"


namespace camera {

struct CameraObject;
COMPONENT_VECTOR(CameraObject, cameras);
struct CameraObject : public ecs::Component {
	CameraObject() : ecs::Component() {
		cameras.push_back(this);
	}
	virtual ~CameraObject() {}

	virtual transform::TransformObject* get_transform() {
		ecs::Entity* e = get_entity();
		return e->get<transform::TransformObject>();
	}
	virtual dynamic::DynamicObject* get_dynamic() {
		ecs::Entity* e = get_entity();
		return e->get<dynamic::DynamicObject>();
	}
	virtual shaders::ShaderUniformsObject* get_uniform() {
		 ecs::Entity* e = get_entity();
		return e->get<shaders::ShaderUniformsObject>();
	}
	DETACH_VECTOR(CameraObject, cameras)
};


}
