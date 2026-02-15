#pragma once

#include "engine/params.h"
#include "engine/params_glm.h"
#include "utils/file_system.hpp"

#include "ambient_layers.hpp"
#include "camera_shake.hpp"
#include "countdown.hpp"
#include "game_over_sequence.hpp"
#include "global_fx.hpp"
#include "lives.hpp"
#include "pause_menu.hpp"
#include "round_transition.hpp"
#include "scoreboard.hpp"

namespace engine::shrooms::config_params {

inline constexpr const char* kConfigPath = "shrooms/config.toml";
inline constexpr const char* kConfigChangedPath = "shrooms/config.changed.toml";

inline void register_params() {
  static bool registered = false;
  if (registered) return;
  registered = true;

  auto& reg = engine::params::registry();

  auto& fx = reg.group("shrooms/global_fx");
  reg.add(fx, "color_tint", global_fx::config.color_tint).label("Color Tint");
  reg.add(fx, "glow_radius_scale", global_fx::config.glow_radius_scale)
      .label("Glow Radius")
      .range(0.0f, 0.3f, 0.001f);
  reg.add(fx, "glow_edge_scale", global_fx::config.glow_edge_scale)
      .label("Glow Edge")
      .range(0.0f, 0.2f, 0.001f);
  reg.add(fx, "glow_edge_power", global_fx::config.glow_edge_power)
      .label("Glow Power")
      .range(0.1f, 3.0f, 0.05f);
  reg.add(fx, "glow_noise_strength", global_fx::config.glow_noise_strength)
      .label("Glow Noise")
      .range(0.0f, 0.5f, 0.005f);
  reg.add(fx, "glow_noise_scale", global_fx::config.glow_noise_scale)
      .label("Noise Scale")
      .range(0.001f, 0.05f, 0.0005f);
  reg.add(fx, "glow_mask_strength", global_fx::config.glow_mask_strength)
      .label("Mask Strength")
      .range(0.0f, 1.0f, 0.01f);
  reg.add(fx, "glow_blur_px", global_fx::config.glow_blur_px)
      .label("Blur Px")
      .range(0.0f, 20.0f, 0.1f);
  reg.add(fx, "glow_divide_strength", global_fx::config.glow_divide_strength)
      .label("Divide Strength")
      .range(0.0f, 1.0f, 0.01f);
  reg.add(fx, "glow_divide_epsilon", global_fx::config.glow_divide_epsilon)
      .label("Divide Eps")
      .range(0.0f, 1.0f, 0.01f);
  reg.add(fx, "shake_pad_px", global_fx::config.shake_pad_px)
      .label("Shake Pad")
      .range(0.0f, 40.0f, 1.0f);
  reg.add(fx, "tint_layer", global_fx::config.tint_layer)
      .label("Tint Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(fx, "glow_layer", global_fx::config.glow_layer)
      .label("Glow Layer")
      .range(-50.0f, 50.0f, 1.0f);

  auto& ambient = reg.group("shrooms/ambient_layers");
  reg.add(ambient, "color", ambient_layers::config.color).label("Color");
  reg.add(ambient, "lifetime", ambient_layers::config.lifetime)
      .label("Lifetime")
      .range(0.2f, 6.0f, 0.05f);
  reg.add(ambient, "spawn_period", ambient_layers::config.spawn_period)
      .label("Spawn Period")
      .range(0.05f, 2.0f, 0.01f);
  reg.add(ambient, "spawn_jitter", ambient_layers::config.spawn_jitter)
      .label("Spawn Jitter")
      .range(0.0f, 1.0f, 0.01f);
  reg.add(ambient, "min_radius", ambient_layers::config.min_radius)
      .label("Min Radius")
      .range(0.0f, 10.0f, 0.1f);
  reg.add(ambient, "max_radius", ambient_layers::config.max_radius)
      .label("Max Radius")
      .range(0.0f, 20.0f, 0.1f);
  reg.add(ambient, "min_speed", ambient_layers::config.min_speed)
      .label("Min Speed")
      .range(-30.0f, 30.0f, 0.5f);
  reg.add(ambient, "max_speed", ambient_layers::config.max_speed)
      .label("Max Speed")
      .range(-30.0f, 30.0f, 0.5f);
  reg.add(ambient, "layer", ambient_layers::config.layer)
      .label("Layer")
      .range(-20.0f, 20.0f, 1.0f);

  auto& bottom_spores = reg.group("shrooms/bottom_spores");
  reg.add(bottom_spores, "color", ambient_layers::bottom_config.color).label("Color");
  reg.add(bottom_spores, "spawn_period", ambient_layers::bottom_config.spawn_period)
      .label("Spawn Period")
      .range(0.05f, 2.0f, 0.01f);
  reg.add(bottom_spores, "spawn_jitter", ambient_layers::bottom_config.spawn_jitter)
      .label("Spawn Jitter")
      .range(0.0f, 1.0f, 0.01f);
  reg.add(bottom_spores, "min_lifetime", ambient_layers::bottom_config.min_lifetime)
      .label("Min Lifetime")
      .range(0.2f, 6.0f, 0.05f);
  reg.add(bottom_spores, "max_lifetime", ambient_layers::bottom_config.max_lifetime)
      .label("Max Lifetime")
      .range(0.2f, 6.0f, 0.05f);
  reg.add(bottom_spores, "min_radius", ambient_layers::bottom_config.min_radius)
      .label("Min Radius")
      .range(0.0f, 20.0f, 0.1f);
  reg.add(bottom_spores, "max_radius", ambient_layers::bottom_config.max_radius)
      .label("Max Radius")
      .range(0.0f, 20.0f, 0.1f);
  reg.add(bottom_spores, "min_up_speed", ambient_layers::bottom_config.min_up_speed)
      .label("Min Up Speed")
      .range(0.0f, 60.0f, 0.5f);
  reg.add(bottom_spores, "max_up_speed", ambient_layers::bottom_config.max_up_speed)
      .label("Max Up Speed")
      .range(0.0f, 60.0f, 0.5f);
  reg.add(bottom_spores, "top_band_fraction", ambient_layers::bottom_config.top_band_fraction)
      .label("Top Band")
      .range(0.02f, 1.0f, 0.01f);
  reg.add(bottom_spores, "layer", ambient_layers::bottom_config.layer)
      .label("Layer")
      .range(-20.0f, 20.0f, 1.0f);

  auto& shake = reg.group("shrooms/camera_shake");
  reg.add(shake, "max_offset_px", camera_shake::config.max_offset_px)
      .label("Max Offset")
      .range(0.0f, 40.0f, 0.5f);
  reg.add(shake, "decay", camera_shake::config.decay)
      .label("Decay")
      .range(0.0f, 10.0f, 0.1f);
  reg.add(shake, "frequency", camera_shake::config.frequency)
      .label("Frequency")
      .range(0.0f, 60.0f, 0.5f);

  auto& round = reg.group("shrooms/round_transition");
  reg.add(round, "overlay_color", round_transition::config.overlay_color).label("Overlay Color");
  reg.add(round, "overlay_fade_in", round_transition::config.overlay_fade_in)
      .label("Overlay In")
      .range(0.0f, 2.0f, 0.01f);
  reg.add(round, "overlay_hold", round_transition::config.overlay_hold)
      .label("Overlay Hold")
      .range(0.0f, 3.0f, 0.01f);
  reg.add(round, "overlay_fade_out", round_transition::config.overlay_fade_out)
      .label("Overlay Out")
      .range(0.0f, 2.0f, 0.01f);
  reg.add(round, "text_delay", round_transition::config.text_delay)
      .label("Text Delay")
      .range(0.0f, 2.0f, 0.01f);
  reg.add(round, "text_fade_in", round_transition::config.text_fade_in)
      .label("Text In")
      .range(0.0f, 2.0f, 0.01f);
  reg.add(round, "text_fade_out", round_transition::config.text_fade_out)
      .label("Text Out")
      .range(0.0f, 2.0f, 0.01f);
  reg.add(round, "text_font_px", round_transition::config.text_font_px)
      .label("Text Size")
      .range(10.0f, 64.0f, 1.0f);
  reg.add(round, "text_color", round_transition::config.text_color).label("Text Color");
  reg.add(round, "overlay_layer", round_transition::config.overlay_layer)
      .label("Overlay Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(round, "text_layer", round_transition::config.text_layer)
      .label("Text Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(round, "ambient_layer", round_transition::config.ambient_layer)
      .label("Ambient Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(round, "ambient_spores", round_transition::config.ambient_spores)
      .label("Ambient Spores")
      .range(0.0f, 30.0f, 1.0f);
  reg.add(round, "ambient_lifetime", round_transition::config.ambient_lifetime)
      .label("Ambient Life")
      .range(0.2f, 6.0f, 0.05f);
  reg.add(round, "ambient_min_radius", round_transition::config.ambient_min_radius)
      .label("Ambient Min")
      .range(0.0f, 20.0f, 0.1f);
  reg.add(round, "ambient_max_radius", round_transition::config.ambient_max_radius)
      .label("Ambient Max")
      .range(0.0f, 30.0f, 0.1f);
  reg.add(round, "ambient_speed", round_transition::config.ambient_speed)
      .label("Ambient Speed")
      .range(0.0f, 60.0f, 0.5f);
  reg.add(round, "ambient_color", round_transition::config.ambient_color).label("Ambient Color");

  auto& game_over = reg.group("shrooms/game_over");
  reg.add(game_over, "overlay_color", game_over_sequence::config.overlay_color)
      .label("Overlay Color");
  reg.add(game_over, "overlay_fade", game_over_sequence::config.overlay_fade)
      .label("Overlay Fade")
      .range(0.0f, 2.0f, 0.01f);
  reg.add(game_over, "text_delay", game_over_sequence::config.text_delay)
      .label("Text Delay")
      .range(0.0f, 3.0f, 0.01f);
  reg.add(game_over, "text_fade", game_over_sequence::config.text_fade)
      .label("Text Fade")
      .range(0.0f, 2.0f, 0.01f);
  reg.add(game_over, "shake_duration", game_over_sequence::config.shake_duration)
      .label("Shake Dur")
      .range(0.0f, 5.0f, 0.05f);
  reg.add(game_over, "total_duration", game_over_sequence::config.total_duration)
      .label("Total Dur")
      .range(0.0f, 8.0f, 0.05f);
  reg.add(game_over, "shake_frequency", game_over_sequence::config.shake_frequency)
      .label("Shake Freq")
      .range(0.0f, 60.0f, 0.5f);
  reg.add(game_over, "shake_amplitude_px", game_over_sequence::config.shake_amplitude_px)
      .label("Shake Amp")
      .range(0.0f, 30.0f, 0.5f);
  reg.add(game_over, "text_font_px", game_over_sequence::config.text_font_px)
      .label("Text Size")
      .range(10.0f, 64.0f, 1.0f);
  reg.add(game_over, "overlay_layer", game_over_sequence::config.overlay_layer)
      .label("Overlay Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(game_over, "text_layer", game_over_sequence::config.text_layer)
      .label("Text Layer")
      .range(0.0f, 200.0f, 1.0f);

  auto& countdown_group = reg.group("shrooms/countdown");
  reg.add(countdown_group, "scale", countdown::config.scale)
      .label("Scale")
      .range(0.05f, 0.4f, 0.01f);
  reg.add(countdown_group, "layer", countdown::config.layer)
      .label("Layer")
      .range(0.0f, 200.0f, 1.0f);

  auto& pause_group = reg.group("shrooms/pause_menu");
  reg.add(pause_group, "overlay_color", pause_menu::config.overlay_color)
      .label("Overlay Color");
  reg.add(pause_group, "menu_color", pause_menu::config.menu_color)
      .label("Menu Color");
  reg.add(pause_group, "countdown_scale", pause_menu::config.countdown_scale)
      .label("Countdown")
      .range(0.05f, 0.4f, 0.01f);
  reg.add(pause_group, "overlay_layer", pause_menu::config.overlay_layer)
      .label("Overlay Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(pause_group, "menu_layer", pause_menu::config.menu_layer)
      .label("Menu Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(pause_group, "button_layer", pause_menu::config.button_layer)
      .label("Button Layer")
      .range(0.0f, 200.0f, 1.0f);
  reg.add(pause_group, "text_layer", pause_menu::config.text_layer)
      .label("Text Layer")
      .range(0.0f, 200.0f, 1.0f);

  auto& lives_group = reg.group("shrooms/lives");
  reg.add(lives_group, "horizontal_spacing", lives::config.horizontal_spacing)
      .label("Heart Spacing")
      .range(0.0f, 0.3f, 0.005f);
  reg.add(lives_group, "hearts_center_offset_x", lives::config.hearts_center_offset.x)
      .label("Hearts Offset X")
      .range(-0.5f, 0.5f, 0.005f);
  reg.add(lives_group, "hearts_center_offset_y", lives::config.hearts_center_offset.y)
      .label("Hearts Offset Y")
      .range(-0.5f, 0.5f, 0.005f);
  reg.add(lives_group, "layer", lives::config.layer)
      .label("Layer")
      .range(0.0f, 200.0f, 1.0f);

  auto& score_group = reg.group("shrooms/scoreboard");
  reg.add(score_group, "row_spacing", scoreboard::config.row_spacing)
      .label("Row Spacing")
      .range(0.0f, 0.2f, 0.005f);
  reg.add(score_group, "text_font_px", scoreboard::config.text_font_px)
      .label("Text Size")
      .range(8.0f, 48.0f, 1.0f);
  reg.add(score_group, "layer", scoreboard::config.layer)
      .label("Layer")
      .range(0.0f, 200.0f, 1.0f);
}

inline void setup_io() {
  const std::string source = file::asset(kConfigPath);
  const std::string autosave = file::asset(kConfigChangedPath);
  engine::params::set_source_file({source, 0.5});
  engine::params::set_autosave_path(autosave);
  engine::params::enable_autosave_on_exit(true);
  engine::params::load_toml(source);
}

}  // namespace engine::shrooms::config_params
