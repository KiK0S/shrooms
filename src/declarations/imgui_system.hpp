#pragma once
#include "../definitions/systems/imgui_system.hpp"

namespace imgui_system {

    void init() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.WantCaptureMouse = true;  // This allows ImGui to process mouse events
        io.WantCaptureKeyboard = true;  // This allows ImGui to process keyboard events

        ImGui::StyleColorsDark();
        
        SDL_Window* window = render_system::_window;
        SDL_GLContext gl_context = SDL_GL_GetCurrentContext();
        
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        const char* glsl_version = "#version 300 es";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }
    ImGuiSystem imgui_renderer;
    init::CallbackOnStart imgui_init(&init);
} 