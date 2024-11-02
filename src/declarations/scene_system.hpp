#pragma once
#include "scene_system.hpp"

namespace scene {

void init() {
    for (auto& it : scene_objects) {
        auto& v = it.second;
        std::sort(v.begin(), v.end(), cmp());
    }
}

init::CallbackOnStart init_scenes(&init, 100);
Scene main("main");

}