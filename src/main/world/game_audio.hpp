#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
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
inline constexpr float kSfxGain = kBgmGain * 0.40f;
inline constexpr float kDefaultMasterGain = 1.0f;
inline constexpr double kCatchMinGapSeconds = 0.04;
inline constexpr double kFamiliarStartMinGapSeconds = 0.10;
inline constexpr double kFamiliarReturnMinGapSeconds = 0.04;
inline constexpr double kMushroomFallMinGapSeconds = 0.14;
inline constexpr double kMushroomShotMinGapSeconds = 0.04;
inline constexpr size_t kCatchSoundCount = 2;

inline bool initialized = false;
inline bool muted = false;
inline bool page_active = true;
inline float master_gain_value = kDefaultMasterGain;

inline engine::SoundId bgm_sound_id = engine::kInvalidSoundId;
inline std::array<engine::SoundId, kCatchSoundCount> catch_sound_ids{
    engine::kInvalidSoundId,
    engine::kInvalidSoundId,
};
inline engine::SoundId familiar_start_sound_id = engine::kInvalidSoundId;
inline engine::SoundId familiar_return_sound_id = engine::kInvalidSoundId;
inline engine::SoundId mushroom_fall_sound_id = engine::kInvalidSoundId;
inline engine::SoundId mushroom_shot_sound_id = engine::kInvalidSoundId;
inline std::uint32_t catch_sound_rng = 0x8c2f3a1du;

inline ecs::Entity* bgm_entity = nullptr;
inline audio_system::AudioObject* bgm_audio = nullptr;
inline ecs::Entity* voice_controller_entity = nullptr;

enum class ManagedSoundKind : size_t {
  Catch = 0,
  FamiliarStart,
  FamiliarReturn,
  MushroomFall,
  MushroomShot,
  Count,
};

inline constexpr size_t kManagedSoundCount = static_cast<size_t>(ManagedSoundKind::Count);

struct ManagedVoice {
  engine::audio::VoiceId voice_id = engine::audio::kInvalidVoiceId;
  engine::SoundId sound_id = engine::kInvalidSoundId;
  ManagedSoundKind kind = ManagedSoundKind::Catch;
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
    std::numeric_limits<double>::lowest(),
};

inline float effective_master_gain() { return muted ? 0.0f : master_gain_value; }

inline void apply_master_gain() { engine::audio::set_master_gain(effective_master_gain()); }

inline size_t managed_sound_index(ManagedSoundKind kind) {
  return static_cast<size_t>(kind);
}

inline double audio_time_seconds() { return ecs::context().time_seconds; }

inline float master_gain() { return master_gain_value; }

inline float master_gain_percent() { return std::round(master_gain_value * 100.0f); }

inline void set_master_gain(float gain) {
  master_gain_value = std::clamp(gain, 0.0f, 1.0f);
  apply_master_gain();
}

inline void sync_master_gain() {
  master_gain_value = std::clamp(master_gain_value, 0.0f, 1.0f);
  apply_master_gain();
}

inline void set_muted(bool value) {
  muted = value;
  apply_master_gain();
}

inline void toggle_muted() { set_muted(!muted); }

inline bool is_muted() { return muted; }

inline std::string audio_toggle_label() { return muted ? "Audio: Muted" : "Audio: On"; }

inline float volume_slider_value() { return muted ? 0.0f : master_gain_value; }

inline std::string volume_label() { return "Volume"; }

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

inline size_t next_catch_sound_index(size_t count) {
  if (count == 0) return 0;
  catch_sound_rng = catch_sound_rng * 1664525u + 1013904223u;
  return static_cast<size_t>((catch_sound_rng >> 16u) % static_cast<std::uint32_t>(count));
}

template <size_t N>
inline void spawn_limited_random_one_shot(ManagedSoundKind kind,
                                          const std::array<engine::SoundId, N>& sound_ids,
                                          float gain, double min_gap_seconds,
                                          size_t max_active_voices) {
  if (!page_active) return;

  std::array<engine::SoundId, N> valid_sound_ids{};
  size_t valid_count = 0;
  for (engine::SoundId sound_id : sound_ids) {
    if (sound_id != engine::kInvalidSoundId) {
      valid_sound_ids[valid_count++] = sound_id;
    }
  }
  if (valid_count == 0) return;
  if (!trigger_allowed(kind, min_gap_seconds)) return;
  if (count_active_voices(kind) >= max_active_voices) return;

  create_managed_voice(kind, valid_sound_ids[next_catch_sound_index(valid_count)], gain, gain);
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

inline void play_mushroom_catch() {
  spawn_limited_random_one_shot(ManagedSoundKind::Catch, catch_sound_ids, kSfxGain,
                                kCatchMinGapSeconds, 3);
}

inline void play_familiar_shot() {
  spawn_limited_one_shot(ManagedSoundKind::FamiliarStart, familiar_start_sound_id, kSfxGain,
                         kFamiliarStartMinGapSeconds, 2);
}

inline void play_familiar_return() {
  spawn_limited_one_shot(ManagedSoundKind::FamiliarReturn, familiar_return_sound_id, kSfxGain,
                         kFamiliarReturnMinGapSeconds, 3);
}

inline void play_mushroom_fall() {
  spawn_limited_one_shot(ManagedSoundKind::MushroomFall, mushroom_fall_sound_id, kSfxGain,
                         kMushroomFallMinGapSeconds, 1);
}

inline void play_mushroom_shot() {
  spawn_limited_one_shot(ManagedSoundKind::MushroomShot, mushroom_shot_sound_id, kSfxGain,
                         kMushroomShotMinGapSeconds, 3);
}

inline void init() {
  if (initialized) {
    apply_master_gain();
    return;
  }
  initialized = true;

  bgm_sound_id =
      register_and_load_sound("shrooms_bgm_forest_night", "shrooms/audio/bgm/69_forest_night.wav");
  catch_sound_ids[0] = register_and_load_sound("shrooms_sfx_mushroom_catch_1",
                                               "shrooms/audio/sfx/mushroom_catch1.wav");
  catch_sound_ids[1] = register_and_load_sound("shrooms_sfx_mushroom_catch_2",
                                               "shrooms/audio/sfx/mushroom_catch2.wav");
  familiar_start_sound_id = register_and_load_sound("shrooms_sfx_familiar_start",
                                                    "shrooms/audio/sfx/familiar_start.wav");
  familiar_return_sound_id = register_and_load_sound("shrooms_sfx_familiar_return",
                                                     "shrooms/audio/sfx/familiar_return.wav");
  mushroom_fall_sound_id = register_and_load_sound("shrooms_sfx_mushroom_fall",
                                                   "shrooms/audio/sfx/mushroom_fall.wav");
  mushroom_shot_sound_id = register_and_load_sound("shrooms_sfx_mushroom_shot",
                                                   "shrooms/audio/mushroom_shot.wav");

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
