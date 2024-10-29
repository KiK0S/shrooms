#pragma once
#include <string>
#include <fstream>
#include <sstream>

namespace file {

std::string asset(std::string name) {
	#ifdef __EMSCRIPTEN__
		return "/assets/" + name;
	#else
		return "../assets/" + name;
	#endif
}

std::string shader(std::string name) {
	#ifdef __EMSCRIPTEN__
		return "/shaders/" + name;
	#else
		return "../shaders/" + name;
	#endif
}

std::string read_file(std::string path) {
	std::ifstream input (path.c_str ());
	if (!input)
		throw std::ios_base::failure ("Error: cannot open " + path);
	std::stringstream buffer;
	buffer << input.rdbuf ();
    return buffer.str ();
}
}