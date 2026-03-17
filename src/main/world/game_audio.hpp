#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "engine/audio.h"
#include "engine/resource_ids.h"
#include "systems/audio/audio_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"

#include "shrooms_assets.hpp"

namespace shrooms::audio {

inline constexpr float kBgmGain = 0.35f;
inline constexpr float kBiteGain = 0.85f;
inline constexpr float kWindGain = 0.45f;
inline constexpr float kExplosionGain = 0.75f;
inline constexpr float kFallNegativeGain = 0.70f;
inline constexpr float kDefaultMasterGain = 1.0f;

inline bool initialized = false;
inline bool muted = false;
inline float master_gain = kDefaultMasterGain;

inline engine::SoundId bgm_sound_id = engine::kInvalidSoundId;
inline engine::SoundId bite_sound_id = engine::kInvalidSoundId;
inline engine::SoundId wind_sound_id = engine::kInvalidSoundId;
inline engine::SoundId explosion_sound_id = engine::kInvalidSoundId;
inline engine::SoundId fall_negative_sound_id = engine::kInvalidSoundId;

inline ecs::Entity* bgm_entity = nullptr;
inline audio_system::AudioObject* bgm_audio = nullptr;
inline ecs::Entity* oneshot_voice_gc_entity = nullptr;
inline std::vector<engine::audio::VoiceId> active_oneshot_voices{};

inline float effective_master_gain() { return muted ? 0.0f : master_gain; }

inline void apply_master_gain() { engine::audio::set_master_gain(effective_master_gain()); }

inline void set_master_gain(float gain) {
  master_gain = gain < 0.0f ? 0.0f : gain;
  apply_master_gain();
}

inline void set_muted(bool value) {
  muted = value;
  apply_master_gain();
}

inline void toggle_muted() { set_muted(!muted); }

inline bool is_muted() { return muted; }

inline std::string audio_toggle_label() { return muted ? "Audio: Muted" : "Audio: On"; }

struct OneShotVoiceGcSystem : public dynamic::DynamicObject {
  OneShotVoiceGcSystem() : dynamic::DynamicObject() {}
  ~OneShotVoiceGcSystem() override { Component::component_count--; }

  void update() override {
    size_t write_index = 0;
    for (size_t i = 0; i < active_oneshot_voices.size(); ++i) {
      const engine::audio::VoiceId voice_id = active_oneshot_voices[i];
      if (voice_id == engine::audio::kInvalidVoiceId) {
        continue;
      }
      if (engine::audio::voice_finished(voice_id)) {
        engine::audio::destroy_voice(voice_id);
        continue;
      }
      active_oneshot_voices[write_index++] = voice_id;
    }
    active_oneshot_voices.resize(write_index);
  }
};

inline engine::SoundId register_and_load_sound(const char* sound_name,
                                               const char* relative_asset_path) {
  const engine::SoundId id = engine::resources::register_sound(sound_name);
  if (id == engine::kInvalidSoundId) {
    std::cerr << "Failed to register sound id for " << sound_name << std::endl;
    return engine::kInvalidSoundId;
  }

  const std::string wav_path = shrooms::asset_path(relative_asset_path);
  if (!engine::audio::load_wav(id, wav_path)) {
    std::cerr << "Failed to load wav at path: " << wav_path << std::endl;
    return engine::kInvalidSoundId;
  }
  return id;
}

inline void spawn_one_shot(engine::SoundId sound_id, float gain) {
  if (sound_id == engine::kInvalidSoundId) return;

  const engine::audio::VoiceId voice_id = engine::audio::create_voice();
  if (voice_id == engine::audio::kInvalidVoiceId) return;
  engine::audio::set_voice_loop(voice_id, false);
  engine::audio::set_voice_gain(voice_id, gain);
  engine::audio::set_voice_sound(voice_id, sound_id, true);
  engine::audio::set_voice_playing(voice_id, true);
  active_oneshot_voices.push_back(voice_id);
}

inline void init() {
  if (initialized) {
    apply_master_gain();
    return;
  }
  initialized = true;

  bgm_sound_id =
      register_and_load_sound("shrooms_bgm_lofi_loop", "shrooms/audio/bgm/lofi_menu_loop.wav");
  bite_sound_id =
      register_and_load_sound("shrooms_sfx_mushroom_bite", "shrooms/audio/sfx/mushroom_bite.wav");
  wind_sound_id = register_and_load_sound("shrooms_sfx_familiar_shot_wind",
                                          "shrooms/audio/sfx/familiar_shot_wind.wav");
  explosion_sound_id = register_and_load_sound(
      "shrooms_sfx_familiar_shot_explosion", "shrooms/audio/sfx/familiar_shot_explosion.wav");
  fall_negative_sound_id =
      register_and_load_sound("shrooms_sfx_fall_negative", "shrooms/audio/sfx/fall_negative_alt.wav");

  if (bgm_sound_id == engine::kInvalidSoundId || bgm_audio) {
    apply_master_gain();
    return;
  }

  bgm_entity = arena::create<ecs::Entity>();
  bgm_audio = arena::create<audio_system::AudioObject>();
  bgm_audio->sound = bgm_sound_id;
  bgm_audio->playing = true;
  bgm_audio->loop = true;
  bgm_audio->gain = kBgmGain;
  bgm_audio->destroy_on_finish = false;
  bgm_entity->add(bgm_audio);

  if (!oneshot_voice_gc_entity) {
    oneshot_voice_gc_entity = arena::create<ecs::Entity>();
    oneshot_voice_gc_entity->add(arena::create<OneShotVoiceGcSystem>());
  }

  apply_master_gain();
}

inline void play_mushroom_bite() { spawn_one_shot(bite_sound_id, kBiteGain); }

inline void play_familiar_shot_wind() { spawn_one_shot(wind_sound_id, kWindGain); }

inline void play_familiar_shot_explosion() {
  spawn_one_shot(explosion_sound_id, kExplosionGain);
}

inline void play_fall_negative() { spawn_one_shot(fall_negative_sound_id, kFallNegativeGain); }

}  // namespace shrooms::audio
