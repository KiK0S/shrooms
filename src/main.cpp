#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include "declarations/all_components.hpp"
#include "declarations/all_systems.hpp"
#include "utils/file_system.hpp"
#include "game_objects/player.hpp"
#include "game_objects/camera.hpp"
#include "world/mushroom_catcher.hpp"
#include <optional>


int main() {
	auto window = window::get_window("lostnfound");
	#ifdef __EMSCRIPTEN__
		window::get_canvas_context();
		SDL_SetWindowFullscreen(window, 0);
		SDL_SetWindowSize(window, 960, 960);
	#endif
	render_system::init(window);
	level_loader::load(file::asset("mushrooms.data"));
	init::init();
	scene::main.activate();
	game_loop::startLoop();
}