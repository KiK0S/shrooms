#pragma once

#include <algorithm>

#include "glm/glm/vec2.hpp"

namespace shrooms::screen {

inline int view_width = 900;
inline int view_height = 900;

inline void set_view(int width, int height) {
  view_width = std::max(1, width);
  view_height = std::max(1, height);
}

inline glm::vec2 norm_to_pixels(const glm::vec2& norm) {
  const float x = (norm.x + 1.0f) * 0.5f * static_cast<float>(view_width);
  const float y = (1.0f - (norm.y + 1.0f) * 0.5f) * static_cast<float>(view_height);
  return glm::vec2{x, y};
}

inline glm::vec2 scale_to_pixels(const glm::vec2& scale) {
  return glm::vec2{scale.x * static_cast<float>(view_width),
                   scale.y * static_cast<float>(view_height)};
}

inline glm::vec2 center_to_top_left(const glm::vec2& center, const glm::vec2& size) {
  return center - size * 0.5f;
}

}  // namespace shrooms::screen
