#include "engine/drivers.h"
#include "engine/input.h"
#include "engine/platform.h"
#include "shrooms_app.hpp"
#include "systems/render/renderable.hpp"

#ifdef __EMSCRIPTEN__
#include "engine/platform_emscripten.h"

#include <emscripten/emscripten.h>
#else
#include "engine/platform_sdl.h"
#endif

#ifdef __EMSCRIPTEN__
namespace {

engine::InputQueue* web_input_queue = nullptr;
engine::EmscriptenPlatform* web_platform = nullptr;

void push_web_key_event(int key_code, bool pressed) {
  if (!web_input_queue) return;
  engine::InputEvent evt{};
  evt.kind = pressed ? engine::InputKind::KeyDown : engine::InputKind::KeyUp;
  evt.key_code = key_code;
  web_input_queue->push(evt);
}

}  // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE void shrooms_push_key_event(int key_code, int pressed) {
  push_web_key_event(key_code, pressed != 0);
}

EMSCRIPTEN_KEEPALIVE void shrooms_set_touchscreen_enabled(int enabled) {
  engine::shrooms::set_touchscreen_enabled(enabled != 0);
}

EMSCRIPTEN_KEEPALIVE void shrooms_request_audio_unlock() {
  if (!web_platform) return;
  web_platform->request_audio_unlock();
}

EMSCRIPTEN_KEEPALIVE int shrooms_is_gameplay_active() {
  return engine::shrooms::is_gameplay_active() ? 1 : 0;
}

}  // extern "C"
#endif

int main() {
  const int view_w = 900;
  const int view_h = 900;
  engine::PlatformConfig config{};
  config.width = view_w;
  config.height = view_h;
  config.title = "Shrooms Demo";
  config.renderer = engine::RendererKind::WebGL;

#ifdef __EMSCRIPTEN__
  engine::EmscriptenPlatform platform{};
#else
  engine::SdlPlatform platform{};
#endif
  engine::InputQueue input{};
#ifdef __EMSCRIPTEN__
  web_input_queue = &input;
  web_platform = &platform;
#endif

  platform.init(config, input);
  auto renderer = platform.create_renderer(config);
  renderer->set_view_size(view_w, view_h);
  renderer->set_clear_color(engine::UIColor{0.05f, 0.05f, 0.08f, 1.0f});
  render_system::set_view_size(static_cast<float>(view_w), static_cast<float>(view_h));

  engine::shrooms::ShroomsLogic logic{view_w, view_h};
  engine::RealtimeDriver driver{logic, *renderer};
  logic.init();

  driver.run_main_loop(platform, input);
  return 0;
}
