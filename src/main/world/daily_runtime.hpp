#pragma once

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string>
#include <string_view>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace daily_runtime {

struct LocalDate {
  int year = 1970;
  int month = 1;
  int day = 1;

  std::string iso_yyyy_mm_dd() const {
    char buf[16] = {};
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    return std::string(buf);
  }
};

inline LocalDate local_calendar_date() {
#ifdef __EMSCRIPTEN__
  const int packed = EM_ASM_INT({
    const now = new Date();
    const y = now.getFullYear();
    const m = now.getMonth() + 1;
    const d = now.getDate();
    return (y << 9) | (m << 5) | d;
  });
  LocalDate out{};
  out.year = packed >> 9;
  out.month = (packed >> 5) & 0x0f;
  out.day = packed & 0x1f;
  return out;
#else
  const std::time_t now = std::time(nullptr);
  std::tm tm_local{};
#if defined(_WIN32)
  localtime_s(&tm_local, &now);
#else
  localtime_r(&now, &tm_local);
#endif
  LocalDate out{};
  out.year = tm_local.tm_year + 1900;
  out.month = tm_local.tm_mon + 1;
  out.day = tm_local.tm_mday;
  return out;
#endif
}

inline uint32_t fnv1a_append(uint32_t hash, std::string_view text) {
  for (char c : text) {
    hash ^= static_cast<uint32_t>(static_cast<unsigned char>(c));
    hash *= 16777619u;
  }
  return hash;
}

inline uint32_t day_hash(std::string_view salt, std::string_view iso_date) {
  uint32_t hash = 2166136261u;
  hash = fnv1a_append(hash, salt);
  hash = fnv1a_append(hash, iso_date);
  return hash;
}

}  // namespace daily_runtime
