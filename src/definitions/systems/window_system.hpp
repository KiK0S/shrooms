#pragma once
#include <SDL2/SDL.h>
#include <string>


#ifdef __EMSCRIPTEN__
#include "emscripten/html5.h"
#include <stdexcept>
#include <string>
#endif

namespace window {

SDL_Window * get_window(std::string title) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

  std::cout << "get_window" << std::endl;
  return SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 960, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
}


#ifdef __EMSCRIPTEN__

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE get_canvas_context(const char *target,
                                                   EmscriptenWebGLContextAttributes *attributes) {
    auto context = emscripten_webgl_create_context(target, attributes);
    auto res = emscripten_webgl_make_context_current(context);
    if (res != EMSCRIPTEN_RESULT_SUCCESS)
      throw std::runtime_error("Failed to make WebGL context the current context");
   	emscripten_webgl_enable_extension(context, "WEBGL_color_buffer_float");
   	emscripten_webgl_enable_extension(context, "EXT_color_buffer_float");

    return context;
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE get_canvas_context(const char *target) {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    return get_canvas_context(target, &attrs);
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE get_canvas_context() { return get_canvas_context("#canvas"); }
#endif

}