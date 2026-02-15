#include "shrooms_app.hpp"

#include "ecs/ecs.hpp"
#include "systems/animation/animation_system.hpp"
#include "systems/collision/collision_system.hpp"
#include "systems/render/render_system.hpp"
#include "systems/spawning/periodic_spawner_system.hpp"
#include "systems/defer/deferred_system.hpp"

#include "world/level_loader.hpp"
#include "world/level_manager.hpp"
#include "world/lives.hpp"
#include "world/camera_shake.hpp"
#include "world/ambient_layers.hpp"
#include "world/global_fx.hpp"
#include "world/game_over_sequence.hpp"
#include "world/round_transition.hpp"
#include "world/menu.hpp"
#include "world/shrooms_assets.hpp"
#include "world/mushroom_catcher.hpp"
#include "world/pause_menu.hpp"
#include "world/player.hpp"
#include "world/tutorial.hpp"
#include "world/scoreboard.hpp"
#include "world/shrooms_scenes.hpp"
#include "world/shrooms_screen.hpp"
#include "world/touchscreen.hpp"
#include "world/config_params.hpp"

#include "engine/params_debug_ui.h"

namespace engine::shrooms {

ShroomsLogic::ShroomsLogic(int view_width, int view_height)
    : view_width_(view_width), view_height_(view_height) {}

void ShroomsLogic::on_init() {
  config_params::register_params();
  config_params::setup_io();

  ::shrooms::register_shrooms_svg_assets();

  ::shrooms::screen::set_view(view_width_, view_height_);
  ::shrooms::scenes::init();

  scoreboard::init();
  levels::initialize();
  level_loader::load_default();

  player::init();
  lives::init();
  camera_shake::init();
  ambient_layers::init();
  global_fx::init();
  game_over_sequence::init();
  round_transition::init();
  tutorial::init();
  menu::init();
  pause_menu::init();

  auto* system_entity = arena::create<ecs::Entity>();
  system_entity->add(arena::create<animation::Animation>());
  system_entity->add(arena::create<collision::CollisionSystem>());
  system_entity->add(arena::create<periodic_spawn::PeriodicSpawnerSystem>());
  system_entity->add(arena::create<deferred::DeferredSystem>());
  system_entity->add(arena::create<render_system::RenderSystem>());

  if (levels::level_finished) {
    if (::shrooms::scenes::menu) {
      ::shrooms::scenes::menu->activate();
      ::shrooms::scenes::menu->set_pause(true);
    }
    if (::shrooms::scenes::main) {
      ::shrooms::scenes::main->set_pause(true);
    }
  }
}

void ShroomsLogic::after_tick(const engine::AppContext& ctx,
                              std::span<const engine::InputEvent> events,
                              engine::Frame& frame) {
  ::global_fx::append_post_process(frame);
#ifndef NDEBUG
  engine::params::poll_source(ctx.time_seconds);
  engine::params::debug_ui::update(engine::params::registry(), events, frame.ui,
                                   static_cast<float>(view_width_),
                                   static_cast<float>(view_height_));
#endif
}

}  // namespace engine::shrooms
