#pragma once
#include "scene_system.hpp"

namespace scene {

void init() {
    for (auto& it : scene_objects) {
        auto& v = it.second;
        std::sort(v.begin(), v.end(), cmp());
    }
    auto menu_it = scenes.find("menu");
    if (menu_it != scenes.end()) {
        menu_it->second->set_pause(true);
    }
}

init::CallbackOnStart init_scenes(&init, 100);
Scene main("main");
Scene menu("menu");

} 
