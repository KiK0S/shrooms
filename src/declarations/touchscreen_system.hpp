#pragma once
#include <SDL2/SDL.h>
#include "../definitions/components/dynamic_object.hpp"
#include "../definitions/systems/easy_drawable_system.hpp"
#include "../definitions/systems/geometry_system.hpp"
#include "../definitions/systems/input_system.hpp"
#include "../definitions/components/touch_object.hpp"
#include "../definitions/components/text_object.hpp"
#include "../definitions/systems/text_system.hpp"
#include "../utils/arena.hpp"

#include <ctime>

namespace touchscreen {

TouchSystem touchscreen_system;

}