#pragma once
#include <vector>
#include "glm/glm/vec2.hpp"
#include "glm/glm/mat4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "../../ecs/component.hpp"
#include <iostream>

namespace transform {

struct TransformObject;

COMPONENT_VECTOR(TransformObject, transforms);

struct TransformObject : public ecs::Component {
	TransformObject() : ecs::Component() {
		transforms.push_back(this);
	}
	virtual ~TransformObject() {
		Component::component_count--;
	}

	virtual glm::vec2 get_pos() {
		return glm::vec2(get_model_matrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	virtual void translate(glm::vec2) = 0;
	virtual void scale(glm::vec2) = 0;
	virtual glm::mat4 get_model_matrix() = 0;
	virtual void rotate(float angle) = 0;
	virtual glm::vec2 transform_point(const glm::vec2& point) {
        glm::vec4 transformed = glm::transpose(get_model_matrix()) * glm::vec4(point, 0.0f, 1.0f);
		return glm::vec2(transformed);
    }
	DETACH_VECTOR(TransformObject, transforms)
};


struct NoRotationTransform: public transform::TransformObject {
	NoRotationTransform(): transform::TransformObject(), pos(0.0f, 0.0f), scale_(1.0f, 1.0f) {}
	NoRotationTransform(glm::vec2 top_left, glm::vec2 bottom_right): NoRotationTransform() {
		scale((bottom_right - top_left) / 2.0f);
		translate(top_left + scale_);
	}
	virtual ~NoRotationTransform() {
		Component::component_count--;
	}

	glm::vec2 pos;
	glm::vec2 scale_;
	virtual glm::vec2 get_pos() {
		return pos;
	}
	virtual void translate(glm::vec2 x) {
		pos += x;
	}
	virtual void scale(glm::vec2 x) {	
		scale_ *= x;
		pos *= x;
	}
	virtual glm::mat4 get_model_matrix() {
		return {
			scale_.x, 0.0f, 0.0f, pos.x,
			0.0f, scale_.y, 0.0f, pos.y,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}
	virtual void rotate(float angle) {}
};

struct LocalRotationTransform: public NoRotationTransform {
	LocalRotationTransform(): NoRotationTransform(), angle(0.0f) {}
	LocalRotationTransform(glm::vec2 top_left, glm::vec2 bottom_right): NoRotationTransform(top_left, bottom_right) {}

	virtual ~LocalRotationTransform() {
		Component::component_count--;
	}
	float angle = 0;
	virtual glm::mat4 get_model_matrix() {
		auto pre_rotation = NoRotationTransform::get_model_matrix();
		auto rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
		return rotation * pre_rotation;
	}
	virtual void rotate(float angle) {
		this->angle += angle;
	}
};

struct Transform2d: public TransformObject {
	Transform2d(): TransformObject() {}
	Transform2d(glm::vec2 top_left, glm::vec2 bottom_right): Transform2d() {
		scale(bottom_right - top_left);
		translate(top_left);
	}
	virtual ~Transform2d() {
		Component::component_count--;
	}
	glm::mat4 get_model_matrix() {
		return mat;
	}
	void translate(glm::vec2 v) {
		mat = glm::translate(mat, glm::vec3(v, 0.0f));
	}
	void scale(glm::vec2 v) {
		mat = glm::scale(mat, glm::vec3(v, 1.0f));
	}
	void rotate(float angle) {
		mat = glm::rotate(mat, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	glm::mat4 mat;
};
}
