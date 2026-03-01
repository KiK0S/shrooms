#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "daily_runtime.hpp"
#include "utils/random.hpp"
#include "utils/save_system.hpp"

namespace leaderboard {

struct Entry {
  std::string name;
  int score = 0;
};

constexpr int kMaxEntries = 8;

enum class Profile {
  Normal,
  Easy,
};

inline std::vector<Entry> entries{};
inline bool loaded = false;
inline Profile current_profile = Profile::Normal;
inline std::string loaded_for_date{};
constexpr int kSaveVersion = 2;

inline const char* key_for_profile(Profile profile) {
  switch (profile) {
    case Profile::Easy:
      return "shrooms_infinite_leaderboard_easy";
    case Profile::Normal:
    default:
      return "shrooms_infinite_leaderboard_normal";
  }
}

inline void set_profile(Profile profile) {
  if (current_profile == profile) return;
  current_profile = profile;
  entries.clear();
  loaded = false;
  loaded_for_date.clear();
}

inline std::string sanitize_name(const std::string& raw) {
  std::string out = raw;
  for (char& ch : out) {
    if (ch == '\t' || ch == '\n' || ch == '\r') {
      ch = ' ';
    }
  }
  auto is_space = [](unsigned char ch) { return std::isspace(ch) != 0; };
  size_t start = 0;
  while (start < out.size() && is_space(static_cast<unsigned char>(out[start]))) {
    start++;
  }
  size_t end = out.size();
  while (end > start && is_space(static_cast<unsigned char>(out[end - 1]))) {
    end--;
  }
  out = out.substr(start, end - start);
  if (out.empty()) {
    return "Player";
  }
  return out;
}

inline void sort_entries() {
  std::sort(entries.begin(), entries.end(),
            [](const Entry& a, const Entry& b) { return a.score > b.score; });
}

inline std::string today_iso_date() {
  return daily_runtime::local_calendar_date().iso_yyyy_mm_dd();
}

inline void trim_line_end(std::string& line) {
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
}

inline std::string serialize(const std::string& date) {
  std::ostringstream out;
  out << "version=" << kSaveVersion << '\n';
  out << "date=" << date << '\n';
  for (const auto& entry : entries) {
    out << entry.name << '\t' << entry.score << '\n';
  }
  return out.str();
}

inline void save() {
  const std::string date = loaded_for_date.empty() ? today_iso_date() : loaded_for_date;
  loaded_for_date = date;
  save::write_text(key_for_profile(current_profile), serialize(date));
}

inline void build_default_entries() {
  static const std::array<const char*, kMaxEntries> kDefaultNames = {
      "Gandalf",
      "Leia",
      "Mario",
      "Zelda",
      "Neo",
      "Ripley",
      "Frodo",
      "Trinity",
  };
  entries.clear();
  entries.reserve(kMaxEntries);
  for (const auto* name : kDefaultNames) {
    Entry entry{};
    entry.name = name;
    entry.score = rnd::get_int(100, 1500);
    entries.push_back(entry);
  }
  sort_entries();
  if (entries.size() > static_cast<size_t>(kMaxEntries)) {
    entries.resize(static_cast<size_t>(kMaxEntries));
  }
}

inline bool load_version_2_entries(const std::string& raw, const std::string& today) {
  std::istringstream lines(raw);

  std::string version_line;
  if (!std::getline(lines, version_line)) {
    return false;
  }
  trim_line_end(version_line);
  if (version_line != "version=2") {
    return false;
  }

  std::string date_line;
  if (!std::getline(lines, date_line)) {
    return false;
  }
  trim_line_end(date_line);
  if (date_line.rfind("date=", 0) != 0) {
    return false;
  }
  const std::string save_date = date_line.substr(5);
  if (save_date != today) {
    return false;
  }

  entries.clear();
  std::string line;
  while (std::getline(lines, line)) {
    trim_line_end(line);
    if (line.empty()) continue;
    const size_t pos = line.find('\t');
    if (pos == std::string::npos) continue;
    std::string name = line.substr(0, pos);
    std::string score_str = line.substr(pos + 1);
    int score = 0;
    std::istringstream score_in(score_str);
    if (!(score_in >> score)) continue;
    name = sanitize_name(name);
    if (name.empty()) continue;
    if (score < 0) score = 0;
    entries.push_back(Entry{name, score});
  }

  if (entries.empty()) {
    return false;
  }

  sort_entries();
  if (entries.size() > static_cast<size_t>(kMaxEntries)) {
    entries.resize(static_cast<size_t>(kMaxEntries));
  }
  return true;
}

inline void load_or_default() {
  const std::string today = today_iso_date();
  if (loaded && loaded_for_date == today) return;

  loaded = false;
  entries.clear();

  bool loaded_saved = false;
  auto saved = save::read_text(key_for_profile(current_profile));
  if (saved) {
    loaded_saved = load_version_2_entries(*saved, today);
    if (!loaded_saved) {
      save::write_text(key_for_profile(current_profile), "");
    }
  }

  if (!loaded_saved) {
    build_default_entries();
    loaded_for_date = today;
    save();
    loaded = true;
    return;
  }

  loaded_for_date = today;
  loaded = true;
}

inline bool qualifies(int score) {
  load_or_default();
  if (score < 0) score = 0;
  if (entries.size() < static_cast<size_t>(kMaxEntries)) {
    return true;
  }
  return score > entries.back().score;
}

inline void insert(const std::string& name, int score) {
  load_or_default();
  Entry entry{};
  entry.name = sanitize_name(name);
  entry.score = std::max(0, score);
  entries.push_back(entry);
  sort_entries();
  if (entries.size() > static_cast<size_t>(kMaxEntries)) {
    entries.resize(static_cast<size_t>(kMaxEntries));
  }
  save();
}

inline const std::vector<Entry>& list() {
  load_or_default();
  return entries;
}

inline std::string current_date() {
  load_or_default();
  return loaded_for_date;
}

}  // namespace leaderboard
