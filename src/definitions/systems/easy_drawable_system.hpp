#pragma once

#include "../components/dynamic_object.hpp"
#include "../components/geometry_object.hpp"
#include "../components/transform_object.hpp"
#include "../components/layered_object.hpp"
#include "../components/textured_object.hpp"
#include "../components/shader_object.hpp"
#include "../components/color_object.hpp"
#include "../../ecs/ecs.hpp"
#include "../../utils/arena.hpp"
#include "../../declarations/shader_system.hpp"
#include "../../declarations/texture_system.hpp"
#include "../../declarations/color_system.hpp"
#include "../../declarations/uniform_system.hpp"

namespace render {

struct SolidDrawable : public ecs::Component {
	geometry::GeometryObject* geometry;
	shaders::Program* program;
	SolidDrawable(geometry::GeometryObject* geometry, shaders::Program* program): program(program), geometry(geometry), layer(1),
		uniforms({&model_matrix}), ecs::Component() {}

	virtual geometry::GeometryObject* get_geometry() {
		return geometry;
	}

	virtual transform::TransformObject* get_transform() {
		return &transform;
	}

	virtual layers::LayeredObject* get_layer() {
		return &layer;
	}

	virtual texture::TexturedObject* get_texture() {
		return &texture::no_texture;
	}

	virtual shaders::ShaderUniformsObject* get_uniform() {
		return &uniforms;
	}

	virtual shaders::Program* get_program() {
		return program;
	}

	virtual color::ColoredObject* get_color() {
		return color;
	}

	virtual void bind(ecs::Entity* entity) {
		get_geometry()->bind(entity);
		get_transform()->bind(entity);
		get_layer()->bind(entity);
		get_uniform()->bind(entity);
		get_color()->bind(entity);
		arena::create<shaders::ProgramArgumentObject>(get_program())->bind(entity);
	}

	transform::NoRotationTransform transform;
	layers::ConstLayer layer;
	shaders::ModelMatrix model_matrix;
	shaders::CombinedUniforms uniforms;
	color::ColoredObject* color = &color::no_color;
};

} // namespace easy_drawable
