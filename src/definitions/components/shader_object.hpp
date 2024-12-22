#pragma once
#include <vector>
#include <GL/gl.h>
#include <string>
#include <map>
#include <functional>
#include "utils/file_system.hpp"
#include "../../ecs/ecs.hpp"

namespace shaders {


struct ShaderUniformsObject;
struct ProgramArgumentObject;
struct Program;


std::vector<Program*> programs;

struct Program {
	Program() {
		programs.push_back(this);
	}
	virtual ~Program() {}

	virtual std::string vertex_shader() = 0;
	virtual std::string fragment_shader() = 0;
	virtual std::string get_name() = 0;
	virtual void reg_uniforms(GLuint program_id) = 0;
};

COMPONENT_MAP(ShaderUniformsObject, shader_uniforms);
struct ShaderUniformsObject: public ecs::Component {
	ShaderUniformsObject(std::vector<Program*> programs): ecs::Component() {
		for (auto* program : programs)
			shader_uniforms[program->get_name()].push_back(this);
	}
	virtual ~ShaderUniformsObject() {
		Component::component_count--;
	}

	virtual void reg_uniforms(GLuint program_id) { }
	DETACH_MAP(ShaderUniformsObject, shader_uniforms)
};

struct ProgramArgumentObject : public ecs::Component {
	ProgramArgumentObject(Program* program): program(program), ecs::Component() {}
	virtual ~ProgramArgumentObject() {
		Component::component_count--;
	}
	virtual Program* get_program() {return program;}
	Program* program;
};



struct TwoShaderProgram : public shaders::Program {
	TwoShaderProgram(std::string path_vertex, std::string path_fragment, std::function<void(GLuint)> reg_uniforms): Program(), name(path_fragment + ":" + path_vertex), reg_uniforms_impl(reg_uniforms) {
		vertex_source = file::read_file(file::shader(path_vertex));
		fragment_source = file::read_file(file::shader(path_fragment));
	}
	TwoShaderProgram(std::string path_vertex, std::string path_fragment): TwoShaderProgram(path_vertex, path_fragment, [](GLuint){}) {}
	std::string vertex_shader() {
		return vertex_source;
	}
	std::string fragment_shader() {
		return fragment_source;
	}
	std::string get_name() {
		return name;
	}
	void reg_uniforms(GLuint program_id) {
		for (auto* object : shaders::shader_uniforms[get_name()]) {
			object->reg_uniforms(program_id);
		}
		reg_uniforms_impl(program_id);
	}
	std::string name;
	std::string vertex_source;
	std::string fragment_source;
	std::function<void(GLuint)> reg_uniforms_impl;
};

}