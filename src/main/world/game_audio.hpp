#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <limits>
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
inline constexpr float kWindCrossfadeSeconds = 0.16f;
inline constexpr double kBiteMinGapSeconds = 0.04;
inline constexpr double kWindMinGapSeconds = 0.10;
inline constexpr double kExplosionMinGapSeconds = 0.08;
inline constexpr double kFallNegativeMinGapSeconds = 0.14;

inline bool initialized = false;
inline bool muted = false;
inline bool page_active = true;
inline float master_gain = kDefaultMasterGain;

inline engine::SoundId bgm_sound_id = engine::kInvalidSoundId;
inline engine::SoundId bite_sound_id = engine::kInvalidSoundId;
inline engine::SoundId wind_sound_id = engine::kInvalidSoundId;
inline engine::SoundId explosion_sound_id = engine::kInvalidSoundId;
inline engine::SoundId fall_negative_sound_id = engine::kInvalidSoundId;

inline ecs::Entity* bgm_entity = nullptr;
inline audio_system::AudioObject* bgm_audio = nullptr;
inline ecs::Entity* voice_controller_entity = nullptr;

enum class ManagedSoundKind : size_t {
  Bite = 0,
  Wind,
  Explosion,
  FallNegative,
  Count,
};

inline constexpr size_t kManagedSoundCount = static_cast<size_t>(ManagedSoundKind::Count);

struct ManagedVoice {
  engine::audio::VoiceId voice_id = engine::audio::kInvalidVoiceId;
  engine::SoundId sound_id = engine::kInvalidSoundId;
  ManagedSoundKind kind = ManagedSoundKind::Bite;
  float current_gain = 0.0f;
  float target_gain = 0.0f;
  float fade_from_gain = 0.0f;
  float fade_elapsed = 0.0f;
  float fade_duration = 0.0f;
  bool destroy_after_fade = false;
  double started_at = 0.0;
};

inline std::vector<ManagedVoice> managed_voices{};
inline std::array<double, kManagedSoundCount> last_trigger_times{
    std::numeric_limits<double>::lowest(),
    std::numeric_limits<double>::lowest(),
    std::numeric_limits<double>::lowest(),
    std::numeric_limits<double>::lowest(),
};

inline float effective_master_gain() { return muted ? 0.0f : master_gain; }

inline void apply_master_gain() { engine::audio::set_master_gain(effective_master_gain()); }

inline size_t managed_sound_index(ManagedSoundKind kind) {
  return static_cast<size_t>(kind);
}

inline double audio_time_seconds() { return ecs::context().time_seconds; }

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

inline ManagedVoice* create_managed_voice(ManagedSoundKind kind, engine::SoundId sound_id,
                                          float initial_gain, float target_gain) {
  if (sound_id == engine::kInvalidSoundId) return nullptr;

  const engine::audio::VoiceId voice_id = engine::audio::create_voice();
  if (voice_id == engine::audio::kInvalidVoiceId) return nullptr;
  engine::audio::set_voice_loop(voice_id, false);
  engine::audio::set_voice_gain(voice_id, initial_gain);
  engine::audio::set_voice_sound(voice_id, sound_id, true);
  engine::audio::set_voice_playing(voice_id, true);
  managed_voices.push_back(ManagedVoice{
      .voice_id = voice_id,
      .sound_id = sound_id,
      .kind = kind,
      .current_gain = initial_gain,
      .target_gain = target_gain,
      .fade_from_gain = initial_gain,
      .fade_elapsed = 0.0f,
      .fade_duration = 0.0f,
      .destroy_after_fade = false,
      .started_at = audio_time_seconds(),
  });
  return &managed_voices.back();
}

inline void begin_fade(ManagedVoice& voice, float target_gain, float duration,
                       bool destroy_after_fade) {
  if (voice.voice_id == engine::audio::kInvalidVoiceId) return;
  if (duration <= 0.0f) {
    voice.current_gain = target_gain;
    voice.target_gain = target_gain;
    voice.fade_from_gain = target_gain;
    voice.fade_elapsed = 0.0f;
    voice.fade_duration = 0.0f;
    voice.destroy_after_fade = destroy_after_fade;
    engine::audio::set_voice_gain(voice.voice_id, target_gain);
    if (destroy_after_fade && target_gain <= 0.0f) {
      engine::audio::destroy_voice(voice.voice_id);
      voice.voice_id = engine::audio::kInvalidVoiceId;
    }
    return;
  }

  voice.fade_from_gain = voice.current_gain;
  voice.target_gain = target_gain;
  voice.fade_elapsed = 0.0f;
  voice.fade_duration = duration;
  voice.destroy_after_fade = destroy_after_fade;
}

inline size_t count_active_voices(ManagedSoundKind kind) {
  size_t count = 0;
  for (const auto& voice : managed_voices) {
    if (voice.voice_id != engine::audio::kInvalidVoiceId && voice.kind == kind &&
        !voice.destroy_after_fade) {
      ++count;
    }
  }
  return count;
}

inline bool trigger_allowed(ManagedSoundKind kind, double min_gap_seconds) {
  const size_t index = managed_sound_index(kind);
  const double now = audio_time_seconds();
  if (now - last_trigger_times[index] < min_gap_seconds) {
    return false;
  }
  last_trigger_times[index] = now;
  return true;
}

inline void spawn_limited_one_shot(ManagedSoundKind kind, engine::SoundId sound_id, float gain,
                                   double min_gap_seconds, size_t max_active_voices) {
  if (!page_active || sound_id == engine::kInvalidSoundId) return;
  if (!trigger_allowed(kind, min_gap_seconds)) return;
  if (count_active_voices(kind) >= max_active_voices) return;
  create_managed_voice(kind, sound_id, gain, gain);
}

inline void spawn_crossfaded_restart(ManagedSoundKind kind, engine::SoundId sound_id, float gain,
                                     double min_gap_seconds, float fade_seconds) {
  if (!page_active || sound_id == engine::kInvalidSoundId) return;
  if (!trigger_allowed(kind, min_gap_seconds)) return;

  for (auto& voice : managed_voices) {
    if (voice.voice_id == engine::audio::kInvalidVoiceId || voice.kind != kind ||
        voice.destroy_after_fade) {
      continue;
    }
    begin_fade(voice, 0.0f, fade_seconds, true);
  }

  ManagedVoice* next = create_managed_voice(kind, sound_id, 0.0f, gain);
  if (!next) return;
  begin_fade(*next, gain, fade_seconds, false);
}

struct OneShotVoiceControllerSystem : public dynamic::DynamicObject {
  OneShotVoiceControllerSystem() : dynamic::DynamicObject() {}
  ~OneShotVoiceControllerSystem() override { Component::component_count--; }

  void update() override {
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    size_t write_index = 0;
    for (size_t i = 0; i < managed_voices.size(); ++i) {
      ManagedVoice voice = managed_voices[i];
      if (voice.voice_id == engine::audio::kInvalidVoiceId) {
        continue;
      }
      if (engine::audio::voice_finished(voice.voice_id)) {
        engine::audio::destroy_voice(voice.voice_id);
        continue;
      }
      if (page_active && voice.fade_duration > 0.0f) {
        voice.fade_elapsed = std::min(voice.fade_elapsed + dt, voice.fade_duration);
        const float t = voice.fade_duration > 0.0f ? voice.fade_elapsed / voice.fade_duration : 1.0f;
        voice.current_gain = voice.fade_from_gain + (voice.target_gain - voice.fade_from_gain) * t;
        engine::audio::set_voice_gain(voice.voice_id, voice.current_gain);
        if (voice.fade_elapsed >= voice.fade_duration) {
          voice.fade_duration = 0.0f;
          voice.fade_elapsed = 0.0f;
          voice.fade_from_gain = voice.current_gain;
          if (voice.destroy_after_fade && voice.target_gain <= 0.0f) {
            engine::audio::destroy_voice(voice.voice_id);
            continue;
          }
        }
      }
      managed_voices[write_index++] = voice;
    }
    managed_voices.resize(write_index);
  }
};

inline void set_page_active(bool active) { page_active = active; }

inline void play_mushroom_bite() {
  spawn_limited_one_shot(ManagedSoundKind::Bite, bite_sound_id, kBiteGain, kBiteMinGapSeconds, 3);
}

inline void play_familiar_shot_wind() {
  spawn_crossfaded_restart(ManagedSoundKind::Wind, wind_sound_id, kWindGain, kWindMinGapSeconds,
                           kWindCrossfadeSeconds);
}

inline void play_familiar_shot_explosion() {
  spawn_limited_one_shot(ManagedSoundKind::Explosion, explosion_sound_id, kExplosionGain,
                         kExplosionMinGapSeconds, 2);
}

inline void play_fall_negative() {
  spawn_limited_one_shot(ManagedSoundKind::FallNegative, fall_negative_sound_id, kFallNegativeGain,
                         kFallNegativeMinGapSeconds, 1);
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

  if (!voice_controller_entity) {
    voice_controller_entity = arena::create<ecs::Entity>();
    voice_controller_entity->add(arena::create<OneShotVoiceControllerSystem>());
  }

  apply_master_gain();
}

}  // namespace shrooms::audio
