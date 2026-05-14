#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <sstream>
#include <string>

#include "systems/input/input_system.hpp"
#include "utils/save_system.hpp"

namespace controls {

enum class Action : size_t {
  MoveLeft = 0,
  MoveRight,
  Shoot,
  Trap,
  Count,
};

inline constexpr size_t kActionCount = static_cast<size_t>(Action::Count);
inline constexpr int kKeyArrowLeftDom = 37;
inline constexpr int kKeyArrowUpDom = 38;
inline constexpr int kKeyArrowRightDom = 39;
inline constexpr int kKeyArrowDownDom = 40;
inline constexpr int kKeyArrowLeftSdl = 1073741904;
inline constexpr int kKeyArrowRightSdl = 1073741903;
inline constexpr int kKeyArrowUpSdl = 1073741906;
inline constexpr int kKeyArrowDownSdl = 1073741905;
inline constexpr const char* kSaveKey = "shrooms_controls_v1";

inline constexpr std::array<int, kActionCount> kDefaultBindings{
    'A',
    'D',
    'W',
    'E',
};

inline std::array<int, kActionCount> bindings = kDefaultBindings;
inline bool mobile_layout = false;

inline size_t index(Action action) { return static_cast<size_t>(action); }

inline int canonical_key_code(int key_code) {
  const int normalized = input::normalize_key_code(key_code);
  switch (normalized) {
    case kKeyArrowLeftSdl:
      return kKeyArrowLeftDom;
    case kKeyArrowRightSdl:
      return kKeyArrowRightDom;
    case kKeyArrowUpSdl:
      return kKeyArrowUpDom;
    case kKeyArrowDownSdl:
      return kKeyArrowDownDom;
    default:
      return normalized;
  }
}

inline bool is_supported_key(int key_code) {
  const int key = canonical_key_code(key_code);
  if (key >= 'A' && key <= 'Z') return true;
  if (key >= '0' && key <= '9') return true;
  return key == kKeyArrowLeftDom || key == kKeyArrowRightDom ||
         key == kKeyArrowUpDom || key == kKeyArrowDownDom;
}

inline std::string key_label(int key_code) {
  const int key = canonical_key_code(key_code);
  if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
    return std::string(1, static_cast<char>(key));
  }
  switch (key) {
    case kKeyArrowLeftDom:
      return "Left";
    case kKeyArrowRightDom:
      return "Right";
    case kKeyArrowUpDom:
      return "Up";
    case kKeyArrowDownDom:
      return "Down";
    default:
      return "?";
  }
}

inline const char* action_label(Action action) {
  switch (action) {
    case Action::MoveLeft:
      return "Move Left";
    case Action::MoveRight:
      return "Move Right";
    case Action::Shoot:
      return "Shoot";
    case Action::Trap:
      return "Bat";
    case Action::Count:
      break;
  }
  return "";
}

inline int bound_key(Action action) {
  const size_t i = index(action);
  if (i >= bindings.size()) return 0;
  return bindings[i];
}

inline std::string bound_key_label(Action action) { return key_label(bound_key(action)); }

inline bool is_mobile_layout() { return mobile_layout; }

inline void set_mobile_layout(bool value) { mobile_layout = value; }

inline int sdl_arrow_key_for_dom(int key_code) {
  switch (canonical_key_code(key_code)) {
    case kKeyArrowLeftDom:
      return kKeyArrowLeftSdl;
    case kKeyArrowRightDom:
      return kKeyArrowRightSdl;
    case kKeyArrowUpDom:
      return kKeyArrowUpSdl;
    case kKeyArrowDownDom:
      return kKeyArrowDownSdl;
    default:
      return 0;
  }
}

inline bool is_down(Action action) {
  const int key = bound_key(action);
  if (input::get_button_state(key)) return true;
  const int sdl_key = sdl_arrow_key_for_dom(key);
  return sdl_key != 0 && input::get_button_state(sdl_key);
}

inline bool has_duplicate(const std::array<int, kActionCount>& values) {
  for (size_t i = 0; i < values.size(); ++i) {
    for (size_t j = i + 1; j < values.size(); ++j) {
      if (values[i] == values[j]) return true;
    }
  }
  return false;
}

inline bool bindings_valid(const std::array<int, kActionCount>& values) {
  return std::all_of(values.begin(), values.end(), is_supported_key) &&
         !has_duplicate(values);
}

inline void save() {
  std::ostringstream out;
  out << "1";
  for (int key : bindings) {
    out << ' ' << key;
  }
  save::write_text(kSaveKey, out.str());
}

inline void reset_to_defaults(bool persist = true) {
  bindings = kDefaultBindings;
  if (persist) save();
}

inline bool load() {
  const auto saved = save::read_text(kSaveKey);
  if (!saved) {
    bindings = kDefaultBindings;
    return false;
  }

  std::istringstream in(*saved);
  int version = 0;
  std::array<int, kActionCount> loaded{};
  if (!(in >> version) || version != 1) {
    reset_to_defaults();
    return false;
  }

  for (int& key : loaded) {
    if (!(in >> key)) {
      reset_to_defaults();
      return false;
    }
    key = canonical_key_code(key);
  }

  if (!bindings_valid(loaded)) {
    reset_to_defaults();
    return false;
  }

  bindings = loaded;
  return true;
}

inline std::optional<Action> action_for_key(int key_code) {
  const int key = canonical_key_code(key_code);
  for (size_t i = 0; i < bindings.size(); ++i) {
    if (bindings[i] == key) {
      return static_cast<Action>(i);
    }
  }
  return std::nullopt;
}

inline bool can_bind(Action action, int key_code) {
  const int key = canonical_key_code(key_code);
  if (!is_supported_key(key)) return false;
  for (size_t i = 0; i < bindings.size(); ++i) {
    if (i == index(action)) continue;
    if (bindings[i] == key) return false;
  }
  return true;
}

inline bool set_binding(Action action, int key_code, bool persist = true) {
  const int key = canonical_key_code(key_code);
  if (!can_bind(action, key)) return false;
  bindings[index(action)] = key;
  if (persist) save();
  return true;
}

}  // namespace controls
