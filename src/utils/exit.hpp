#pragma once
#include <stdexcept>

namespace game_over {

void success() {
	#ifdef __EMSCRIPTEN__
		throw std::runtime_error("game ended");
	#else 
		exit(0);
	#endif
		
}

}