#pragma once

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <stdexcept>
#include "../definitions/components/dynamic_object.hpp"
#include "../definitions/systems/gpu_program_system.hpp"
#include "../definitions/systems/texture_system.hpp"
#include "../definitions/systems/color_system.hpp"
#include "../definitions/components/hidden_object.hpp"


namespace render_system {


FrameStartSystem start_system;
FrameEndSystem end_system;

void init_objects() {
	for (auto* geometry : geometry::geometry_objects) {
		add_to_frame(geometry);
	}
}
init::CallbackOnStart init_rendering(init_objects, -1);


}