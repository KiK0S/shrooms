#pragma once

#include <chrono>
#include <functional>
#include "../components/dynamic_object.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#endif

namespace game_loop {

bool isRunning = true;

void Loop() {
    #ifdef __EMSCRIPTEN__
    try {
    #endif

    if (!isRunning) {
        return;
    }

    for (int i = 0; i < dynamic::dynamics.size(); i++) {
        dynamic::dynamics[i]->update();
    }
    
    #ifdef __EMSCRIPTEN__
    } catch(...) {
        emscripten_cancel_main_loop();
        // emscripten_force_exit(1);
    }
    #endif

}


#ifdef __EMSCRIPTEN__

void startLoop() {
    emscripten_set_main_loop(Loop, 0, 1);
}

#else
void startLoop() {
    while (true) {
        Loop();
    }
}
#endif

}