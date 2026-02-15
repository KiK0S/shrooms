#include "engine/drivers.h"
#include "engine/input.h"
#include "engine/platform.h"
#include "engine/platform_sdl.h"
#include "shrooms_app.hpp"
#include "systems/render/renderable.hpp"

int main() {
  const int view_w = 900;
  const int view_h = 900;
  engine::PlatformConfig config{};
  config.width = view_w;
  config.height = view_h;
  config.title = "Shrooms Demo";
  config.renderer = engine::RendererKind::WebGL;

  engine::SdlPlatform platform{};
  engine::InputQueue input{};

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
