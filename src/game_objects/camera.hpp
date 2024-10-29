#pragma once
#include "../definitions/components/shader_object.hpp"
#include "../definitions/components/transform_object.hpp"
#include "glm/gtx/norm.hpp"

namespace camera {


transform::NoRotationTransform camera_transform;
shaders::ViewMatrix view_matrix_provider;

ecs::Entity camera;

void init() {
	std::cout << "init camera\n";
	camera.add(&camera_transform)
				.add(&view_matrix_provider)
				.bind();
} 

init::CallbackOnStart camera_init(&init);


}