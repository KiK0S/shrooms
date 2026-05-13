#include "shrooms_app.hpp"

#include "ecs/ecs.hpp"
#include "systems/animation/animation_system.hpp"
#include "systems/audio/audio_system.hpp"
#include "systems/collision/collision_system.hpp"
#include "systems/render/render_system.hpp"
#include "systems/spawning/periodic_spawner_system.hpp"
#include "systems/defer/deferred_system.hpp"

#include "world/level_loader.hpp"
#include "world/level_manager.hpp"
#include "world/controls.hpp"
#include "world/score_hud.hpp"
#include "world/camera_shake.hpp"
#include "world/ambient_layers.hpp"
#include "world/global_fx.hpp"
#include "world/game_over_sequence.hpp"
#include "world/round_transition.hpp"
#include "world/menu.hpp"
#include "world/shrooms_assets.hpp"
#include "world/game_audio.hpp"
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

namespace {

bool page_active = true;
bool gameplay_auto_paused_by_page = false;

void apply_page_active_state() {
  ::shrooms::audio::set_page_active(page_active);

  auto* main_scene = ::shrooms::scenes::main;
  if (!main_scene) {
    return;
  }

  auto* active_scene = scene::get_active_scene();
  if (!page_active) {
    if (active_scene == main_scene && !main_scene->is_paused_state()) {
      main_scene->set_pause(true);
      gameplay_auto_paused_by_page = true;
    }
    return;
  }

  if (!gameplay_auto_paused_by_page) {
    return;
  }

  gameplay_auto_paused_by_page = false;
  if (active_scene == main_scene && main_scene->is_paused_state()) {
    main_scene->set_pause(false);
  }
}

}  // namespace

ShroomsLogic::ShroomsLogic(int view_width, int view_height)
    : view_width_(view_width), view_height_(view_height) {}

bool is_gameplay_active() {
  auto* main_scene = ::shrooms::scenes::main;
  auto* active_scene = scene::get_active_scene();
  return main_scene && active_scene == main_scene && !scene::is_current_scene_paused();
}

bool is_shoot_enabled() {
  return ::levels::shooting_enabled();
}

int action_key_code(int action_index) {
  if (action_index < 0 ||
      action_index >= static_cast<int>(::controls::kActionCount)) {
    return 0;
  }
  return ::controls::bound_key(static_cast<::controls::Action>(action_index));
}

void set_page_active(bool active) {
  page_active = active;
  apply_page_active_state();
}

void set_touchscreen_enabled(bool enabled) {
  touchscreen::set_enabled(enabled);
}

void set_mobile_layout(bool enabled) {
  ::controls::set_mobile_layout(enabled);
}

void ShroomsLogic::on_init() {
  ::controls::load();
  config_params::register_params();
  config_params::setup_io();

  ::shrooms::register_shrooms_svg_assets();
  ::shrooms::audio::init();

  ::shrooms::screen::set_view(view_width_, view_height_);
  ::shrooms::scenes::init();

  scoreboard::init();
  levels::initialize();
  level_loader::load_default();

  player::init();
  score_hud::init();
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
  system_entity->add(arena::create<audio_system::AudioSyncSystem>());
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

  apply_page_active_state();
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
  ::shrooms::audio::sync_master_gain();
}

}  // namespace engine::shrooms
