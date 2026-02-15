#pragma once

#include "systems/scene/scene_system.hpp"

namespace shrooms::scenes {

inline scene::Scene* main = nullptr;
inline scene::Scene* menu = nullptr;

inline void init() {
  main = scene::ensure_scene("main");
  menu = scene::ensure_scene("menu");
}

}  // namespace shrooms::scenes
