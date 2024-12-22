#include "../components/transform_object.hpp"
#include "../../declarations/shader_system.hpp"

namespace shaders {
struct ModelMatrix : public ShaderUniformsObject {
	ModelMatrix(): ShaderUniformsObject({}) {
		global = true;
	}
	~ModelMatrix(){
		Component::component_count--;
	}

	void reg_uniforms(GLuint program_id) {
		ecs::Entity* e = get_entity();
		transform::TransformObject* t = e->get<transform::TransformObject>();
		auto modelLocation = glGetUniformLocation(program_id, "uModelMatrix");
		glm::mat4 modelMatrix = t->get_model_matrix();
		glUniformMatrix4fv(modelLocation, 1, GL_TRUE, glm::value_ptr(modelMatrix));
	}

};


struct NoShaderUniforms: public ShaderUniformsObject {
	NoShaderUniforms(): ShaderUniformsObject({}) {
		global = true;
	}
	~NoShaderUniforms() {
		Component::component_count--;
	}
	void reg_uniforms(GLuint program_id) { }
};


struct MiniMapUniforms: public ShaderUniformsObject {
	MiniMapUniforms(): ShaderUniformsObject({}) {}
	~MiniMapUniforms() {
		Component::component_count--;
	}
	void reg_uniforms(GLuint p) {
		glUniform1i(glGetUniformLocation(p, "toView"), false);
	}
	void bind(ecs::Entity* e) {}
};


struct CombinedUniforms: public ShaderUniformsObject {
	CombinedUniforms(std::vector<ShaderUniformsObject*> uniforms): uniforms(std::move(uniforms)), ShaderUniformsObject({}) {}

	virtual ~CombinedUniforms() {
		Component::component_count -= uniforms.size();
	}

	void reg_uniforms(GLuint program_id) {
		for (const auto& obj : uniforms) {
			obj->reg_uniforms(program_id);
		}
	}
	void bind(ecs::Entity* e) {
		for (const auto& obj : uniforms) {
			obj->bind(e);
		}
	}
	void add(ShaderUniformsObject* x) {
		uniforms.push_back(x);
	}
	std::vector<ShaderUniformsObject*> uniforms;
};

struct ViewMatrix : public ShaderUniformsObject {
	ViewMatrix() : ShaderUniformsObject({&static_object_program, &raycast_program,  &bezier_raycast_program, &bezier_program, &visibility_program}) {}
	virtual ~ViewMatrix() {
		Component::component_count--;
	}
	void reg_uniforms(GLuint program) {

		ecs::Entity* e = get_entity();
		transform::TransformObject* tr = e->get<transform::TransformObject>();

		auto viewLocation = glGetUniformLocation(program, "uViewMatrix");
		glm::mat4 viewMatrix = tr->get_model_matrix();
		viewMatrix[0][3] *= -1;
		viewMatrix[1][3] *= -1;
		glUniformMatrix4fv(viewLocation, 1, GL_TRUE, glm::value_ptr(viewMatrix));
	}
};
}