#pragma once

#include <iostream>
#include <string>

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "engine/audio.h"
#include "engine/resource_ids.h"
#include "systems/audio/audio_system.hpp"

#include "shrooms_assets.hpp"

namespace shrooms::audio {

inline constexpr float kBgmGain = 0.35f;
inline constexpr float kBiteGain = 0.85f;
inline constexpr float kWindGain = 0.45f;
inline constexpr float kExplosionGain = 0.75f;
inline constexpr float kFallNegativeGain = 0.70f;

inline bool initialized = false;

inline engine::SoundId bgm_sound_id = engine::kInvalidSoundId;
inline engine::SoundId bite_sound_id = engine::kInvalidSoundId;
inline engine::SoundId wind_sound_id = engine::kInvalidSoundId;
inline engine::SoundId explosion_sound_id = engine::kInvalidSoundId;
inline engine::SoundId fall_negative_sound_id = engine::kInvalidSoundId;

inline ecs::Entity* bgm_entity = nullptr;
inline audio_system::AudioObject* bgm_audio = nullptr;

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

  auto* entity = arena::create<ecs::Entity>();
  auto* audio_obj = arena::create<audio_system::AudioObject>();
  audio_obj->sound = sound_id;
  audio_obj->playing = true;
  audio_obj->loop = false;
  audio_obj->gain = gain;
  audio_obj->destroy_on_finish = true;
  entity->add(audio_obj);
}

inline void init() {
  if (initialized) return;
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

  if (bgm_sound_id == engine::kInvalidSoundId || bgm_audio) return;

  bgm_entity = arena::create<ecs::Entity>();
  bgm_audio = arena::create<audio_system::AudioObject>();
  bgm_audio->sound = bgm_sound_id;
  bgm_audio->playing = true;
  bgm_audio->loop = true;
  bgm_audio->gain = kBgmGain;
  bgm_audio->destroy_on_finish = false;
  bgm_entity->add(bgm_audio);
}

inline void play_mushroom_bite() { spawn_one_shot(bite_sound_id, kBiteGain); }

inline void play_familiar_shot_wind() { spawn_one_shot(wind_sound_id, kWindGain); }

inline void play_familiar_shot_explosion() {
  spawn_one_shot(explosion_sound_id, kExplosionGain);
}

inline void play_fall_negative() { spawn_one_shot(fall_negative_sound_id, kFallNegativeGain); }

}  // namespace shrooms::audio
