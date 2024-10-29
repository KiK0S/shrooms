#pragma once
#include "../definitions/systems/dynamic_system.hpp"

namespace dynamic {

void init() {
    std::sort(dynamics.begin(), dynamics.end(), cmp());
}

init::CallbackOnStart init_dynamics(&init);

}