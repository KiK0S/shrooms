#pragma once
#include "../components/configurable_object.hpp"
#include "../components/observable_object.hpp"
#include "../components/dynamic_object.hpp"
#include "../components/controllable_object.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

namespace imgui_system {

struct ImGuiSystem : public dynamic::DynamicObject,  public input::ControllableObject {
    ImGuiSystem() : dynamic::DynamicObject(999), input::ControllableObject() {} // High priority to render after everything else
    virtual ~ImGuiSystem() {
        Component::component_count--;
    }
    void handle_user_action(SDL_Event event) override {
        ImGui_ImplSDL2_ProcessEvent(&event);
    }
    void update() {
        #ifndef __EMSCRIPTEN__
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Configuration")) {
            for (auto* configurable : config::configurables) {
                configurable->register_imgui();
            }
        }
        ImGui::End();

        if (ImGui::Begin("Debug Info")) {
            for (auto* observable : debug::observables) {
                observable->register_imgui();
            }
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #endif
    }
};

} // namespace imgui_system 