#pragma once
#include "../components/shader_object.hpp"
#include <iostream>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glew.h>
#include <functional>
#include <stdexcept>
#include "../../utils/file_system.hpp"
#include "../../declarations/logger_system.hpp"

namespace shaders {

std::map<std::string, GLuint> program_ids;
GLuint load_shader(std::string source, GLenum shader_type);
char errorLog[1024];

void init() {
	for (Program* program_ptr : shaders::programs) {
		LOG_IF(logger::enable_gpu_program_system_logging, "load_program " << program_ptr->get_name());
		GLuint vertexShader = load_shader(program_ptr->vertex_shader(), GL_VERTEX_SHADER);
		GLuint fragmentShader = load_shader(program_ptr->fragment_shader(), GL_FRAGMENT_SHADER);
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		GLint linkSuccess = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
		if (linkSuccess == GL_FALSE) {
				// fail to compile program
				GLint maxLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

				glGetProgramInfoLog(program, maxLength, &maxLength, errorLog);

				for (int i = 0; i < maxLength; i++) {
					std::cerr << errorLog[i];
				}
				std::cerr << '\n';
				glDeleteProgram(program);			
				throw std::runtime_error("cant create program");
		}

		// GLint is_valid = 0;
		// glGetProgramiv(program, GL_VALIDATE_STATUS, &is_valid);
		// if (is_valid == GL_FALSE) {

		// 	std::cerr << "Program validation failed: " << std::endl;
		// 	glDeleteProgram(program);
		// 	throw std::runtime_error("cant create program");
		// }
		program_ids[program_ptr->get_name()] = program;
	}	
}

// GL_VERTEX_SHADER
// GL_FRAGMENT_SHADER
GLuint load_shader(std::string source, GLenum shader_type) {
	LOG_IF(logger::enable_gpu_program_system_logging, "load_shader " << shader_type << " loading");
	GLuint glShader = glCreateShader(shader_type);
	GLchar const *files[] = {source.c_str()};
	GLint lengths[] = {GLint(source.size())};
	glShaderSource(glShader, 1, files, lengths);
	glCompileShader(glShader);
	GLint compileSuccess = 0;
	glGetShaderiv(glShader, GL_COMPILE_STATUS, &compileSuccess);
	if (compileSuccess == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(glShader, GL_INFO_LOG_LENGTH, &maxLength);

			glGetShaderInfoLog(glShader, maxLength, &maxLength, errorLog);
			for (int i = 0; i < maxLength; i++) {
				std::cerr << errorLog[i];
			}
			std::cerr << '\n';
			std::cerr << "Compiling shader failed: " << std::endl;

			// fail to compile shader!
			glDeleteShader(glShader);
			throw std::runtime_error("can't compile shader");
	}
	LOG_IF(logger::enable_gpu_program_system_logging, "load_shader " << shader_type << " loaded");
	return glShader;
}

void run_with_program(Program* program, std::function<void(GLuint)> action) {
	GLuint program_id = program_ids[program->get_name()];
	glUseProgram(program_id);
	action(program_id);
	glUseProgram(0);
}

}