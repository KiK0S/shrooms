#pragma once

#include "../definitions/systems/color_system.hpp"

namespace color {

NoColorObject no_color;

OneColor white({1.0f, 1.0f, 1.0f, 1.0f});
OneColor red({1.0f, 0.0f, 0.0f, 1.0f});
OneColor green({0.0f, 1.0f, 0.0f, 1.0f});
OneColor blue({0.0f, 0.0f, 1.0f, 1.0f});
OneColor light_grey({0.5f, 0.5f, 0.5f, 0.5f});
OneColor lighter_grey({0.5f, 0.5f, 0.5f, 0.3f});
OneColor pink({191.0 / 256, 4.0 / 256, 151.0 / 256, 1.0f});

}