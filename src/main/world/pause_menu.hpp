#pragma once

#include <array>
#include <optional>
#include <string>
#include <utility>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "camera_shake.hpp"
#include "countdown.hpp"
#include "level_manager.hpp"
#include "lives.hpp"
#include "menu.hpp"
#include "player.hpp"
#include "round_transition.hpp"
#include "shrooms_screen.hpp"
#include "vfx.hpp"

namespace pause_menu {

constexpr int kKeyArrowUpDom = 38;
constexpr int kKeyArrowDownDom = 40;
constexpr int kKeyArrowUpSdl = 1073741906;
constexpr int kKeyArrowDownSdl = 1073741905;

struct ActionTextColor : public color::ColoredObject {
  explicit ActionTextColor(const glm::vec4& value) : color::ColoredObject(), value(value) {}
  ~ActionTextColor() override { Component::component_count--; }
  glm::vec4 get_color() override { return value; }
  glm::vec4 value{1.0f, 1.0f, 1.0f, 1.0f};
};

struct Config {
  glm::vec2 overlay_scale = glm::vec2(1.0f, 1.0f);
  glm::vec2 menu_scale = glm::vec2(0.15f, 0.15f);
  glm::vec2 menu_position = glm::vec2(0.0f, 0.0f);
  glm::vec4 overlay_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.33f);
  glm::vec4 menu_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  glm::vec2 resume_scale = glm::vec2(0.25f, 0.08f);
  glm::vec2 resume_offset = glm::vec2(0.0f, -0.03f);
  glm::vec2 restart_scale = glm::vec2(0.25f, 0.08f);
  glm::vec2 restart_offset = glm::vec2(0.0f, -0.2f);
  glm::vec2 main_menu_scale = glm::vec2(0.25f, 0.08f);
  glm::vec2 main_menu_offset = glm::vec2(0.0f, -0.37f);
  glm::vec2 pause_button_position = glm::vec2(0.88f, -0.84f);
  glm::vec2 pause_button_scale = glm::vec2(0.11f, 0.11f);
  glm::vec2 pause_icon_scale = glm::vec2(0.065f, 0.065f);
  float countdown_scale = 0.15f;
  int overlay_layer = 100;
  int menu_layer = 101;
  int button_layer = 102;
  int text_layer = 103;
  int pause_button_layer = 104;
} config;

struct ActionLine {
  ecs::Entity* button_entity = nullptr;
  transform::NoRotationTransform* button_transform = nullptr;
  render_system::QuadRenderable* button_quad = nullptr;
  hidden::HiddenObject* button_hidden = nullptr;

  ecs::Entity* text_entity = nullptr;
  transform::NoRotationTransform* text_transform = nullptr;
  text::TextObject* text_object = nullptr;
  hidden::HiddenObject* text_hidden = nullptr;
  ActionTextColor* text_color = nullptr;

  glm::vec2 button_base_pos{0.0f, 0.0f};
  glm::vec2 button_base_size{0.0f, 0.0f};
  engine::UIColor base_color{0.15f, 0.15f, 0.15f, 0.95f};
  engine::UIColor hover_color{0.28f, 0.2f, 0.35f, 0.98f};
  engine::UIColor selected_color{0.15f, 0.15f, 0.15f, 0.95f};
  engine::UIColor dimmed_color{0.08f, 0.08f, 0.08f, 0.52f};
  glm::vec4 base_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 selected_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 dimmed_text_color{0.72f, 0.72f, 0.72f, 0.92f};
  float hover_scale = 1.05f;
  float selected_scale = 1.0f;
};

inline constexpr size_t kResumeAction = 0;
inline constexpr size_t kRestartAction = 1;
inline constexpr size_t kMainMenuAction = 2;
inline constexpr size_t kActionCount = 3;

inline ecs::Entity* overlay = nullptr;
inline ecs::Entity* menu_window = nullptr;
inline std::array<ActionLine, kActionCount> action_lines{};

inline ecs::Entity* pause_toggle_button = nullptr;
inline transform::NoRotationTransform* pause_toggle_transform = nullptr;
inline render_system::QuadRenderable* pause_toggle_quad = nullptr;
inline hidden::HiddenObject* pause_toggle_button_hidden = nullptr;
inline glm::vec2 pause_toggle_base_pos{0.0f, 0.0f};
inline glm::vec2 pause_toggle_base_size{0.0f, 0.0f};

inline ecs::Entity* pause_toggle_icon = nullptr;
inline render_system::SpriteRenderable* pause_toggle_icon_sprite = nullptr;
inline hidden::HiddenObject* pause_toggle_icon_hidden = nullptr;

inline hidden::HiddenObject* overlay_hidden = nullptr;
inline hidden::HiddenObject* menu_hidden = nullptr;
inline bool pause_menu_open = false;

inline engine::UIColor pause_toggle_base_color{0.15f, 0.15f, 0.15f, 0.92f};
inline engine::UIColor pause_toggle_hover_color{0.22f, 0.22f, 0.22f, 0.98f};
inline float pause_toggle_hover_scale = 1.05f;

inline bool is_arrow_up_key(int key) {
  return key == kKeyArrowUpDom || key == kKeyArrowUpSdl;
}

inline bool is_arrow_down_key(int key) {
  return key == kKeyArrowDownDom || key == kKeyArrowDownSdl;
}

inline bool is_confirm_key(int key) {
  return key == ' ' || key == '\r' || key == '\n' || key == 13;
}

inline bool point_in_bounds(const glm::vec2& point, const glm::vec2& min_bound,
                            const glm::vec2& max_bound) {
  if (point.x < min_bound.x || point.x > max_bound.x) return false;
  if (point.y < min_bound.y || point.y > max_bound.y) return false;
  return true;
}

inline bool is_main_scene_active() {
  auto* active = scene::get_active_scene();
  return active && active->get_name() == "main";
}

inline bool pause_controls_blocked() {
  return levels::has_pending_failure() || levels::level_finished || round_transition::is_active();
}

inline void set_action_visibility(ActionLine& action, bool visible) {
  if (action.button_hidden) {
    action.button_hidden->set_visible(visible);
  }
  if (action.text_hidden) {
    action.text_hidden->set_visible(visible);
  }
}

inline void set_pause_menu_visible(bool visible) {
  pause_menu_open = visible;
  // Keep the pause card hidden: pause actions now follow the plain list style.
  if (menu_hidden) menu_hidden->hide();
  for (auto& action : action_lines) {
    set_action_visibility(action, visible);
  }
}

inline void set_pause_toggle_visible(bool visible) {
  if (pause_toggle_button_hidden) {
    pause_toggle_button_hidden->set_visible(visible);
  }
  if (pause_toggle_icon_hidden) {
    pause_toggle_icon_hidden->set_visible(visible);
  }
}

inline void apply_action_visual(ActionLine& action, float scale, const engine::UIColor& color) {
  if (!action.button_transform || !action.button_quad) return;
  const glm::vec2 size = action.button_base_size * scale;
  const glm::vec2 center = action.button_base_pos + action.button_base_size * 0.5f;
  action.button_transform->pos = center - size * 0.5f;
  action.button_quad->width = size.x;
  action.button_quad->height = size.y;
  action.button_quad->color = color;
}

inline void set_action_visual_state(ActionLine& action, bool selected, bool hovered, bool dimmed) {
  if (action.text_color) {
    action.text_color->value = selected ? action.selected_text_color
                                        : (dimmed ? action.dimmed_text_color
                                                  : action.base_text_color);
  }
  if (selected) {
    apply_action_visual(action, action.selected_scale, action.selected_color);
    return;
  }
  if (hovered) {
    apply_action_visual(action, action.hover_scale, action.hover_color);
    return;
  }
  if (dimmed) {
    apply_action_visual(action, 1.0f, action.dimmed_color);
    return;
  }
  apply_action_visual(action, 1.0f, action.base_color);
}

inline std::pair<glm::vec2, glm::vec2> action_bounds(const ActionLine& action) {
  const glm::vec2 min_bound = action.button_base_pos;
  const glm::vec2 max_bound = action.button_base_pos + action.button_base_size;
  return {min_bound, max_bound};
}

inline std::pair<glm::vec2, glm::vec2> pause_toggle_bounds() {
  const glm::vec2 min_bound = pause_toggle_base_pos;
  const glm::vec2 max_bound = pause_toggle_base_pos + pause_toggle_base_size;
  return {min_bound, max_bound};
}

inline void apply_pause_toggle_hover(bool hovered) {
  if (pause_toggle_transform && pause_toggle_quad) {
    const float scale = hovered ? pause_toggle_hover_scale : 1.0f;
    const glm::vec2 size = pause_toggle_base_size * scale;
    const glm::vec2 center = pause_toggle_base_pos + pause_toggle_base_size * 0.5f;
    pause_toggle_transform->pos = center - size * 0.5f;
    pause_toggle_quad->width = size.x;
    pause_toggle_quad->height = size.y;
    pause_toggle_quad->color = hovered ? pause_toggle_hover_color : pause_toggle_base_color;
  }
  if (pause_toggle_icon_sprite) {
    pause_toggle_icon_sprite->tint =
        hovered ? engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f}
                : engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f};
  }
}

inline ActionLine make_action_line(const std::string& label, const glm::vec2& center_norm,
                                   const glm::vec2& button_scale) {
  ActionLine action{};
  const glm::vec2 center = shrooms::screen::norm_to_pixels(center_norm);
  const glm::vec2 button_size = shrooms::screen::scale_to_pixels(button_scale);

  action.button_entity = arena::create<ecs::Entity>();
  action.button_transform = arena::create<transform::NoRotationTransform>();
  action.button_transform->pos = shrooms::screen::center_to_top_left(center, button_size);
  action.button_base_pos = action.button_transform->pos;
  action.button_base_size = button_size;
  action.button_entity->add(action.button_transform);
  action.button_entity->add(arena::create<layers::ConstLayer>(config.button_layer));
  action.button_quad = arena::create<render_system::QuadRenderable>(
      button_size.x, button_size.y, action.base_color);
  action.button_entity->add(action.button_quad);
  action.button_hidden = arena::create<hidden::HiddenObject>();
  action.button_entity->add(action.button_hidden);
  action.button_hidden->hide();
  action.button_entity->add(arena::create<scene::SceneObject>("main"));

  action.text_entity = arena::create<ecs::Entity>();
  action.text_transform = arena::create<transform::NoRotationTransform>();
  const float font_px = 24.0f;
  const auto layout = engine::text::layout_text(label, 0.0f, 0.0f, font_px);
  const glm::vec2 text_size{layout.width, layout.height};
  action.text_transform->pos = shrooms::screen::center_to_top_left(center, text_size);
  action.text_entity->add(action.text_transform);
  action.text_entity->add(arena::create<layers::ConstLayer>(config.text_layer));
  action.text_object = arena::create<text::TextObject>(label, font_px);
  action.text_entity->add(action.text_object);
  action.text_color = arena::create<ActionTextColor>(action.base_text_color);
  action.text_entity->add(action.text_color);
  action.text_hidden = arena::create<hidden::HiddenObject>();
  action.text_entity->add(action.text_hidden);
  action.text_hidden->hide();
  action.text_entity->add(arena::create<scene::SceneObject>("main"));

  return action;
}

struct PauseMenuController : public dynamic::DynamicObject {
  PauseMenuController() : dynamic::DynamicObject() {}
  ~PauseMenuController() override { Component::component_count--; }

  void update_pointer() {
    for (const auto& evt : input::events()) {
      if (evt.kind == engine::InputKind::PointerMove ||
          evt.kind == engine::InputKind::PointerDown) {
        last_pointer = glm::vec2{static_cast<float>(evt.x), static_cast<float>(evt.y)};
      }
    }
  }

  void move_selected_action(int direction) {
    if (direction == 0) return;
    const int count = static_cast<int>(kActionCount);
    int next = static_cast<int>(selected_action_index) + (direction > 0 ? 1 : -1);
    while (next < 0) {
      next += count;
    }
    next %= count;
    selected_action_index = static_cast<size_t>(next);
  }

  void apply_action_visuals(std::optional<size_t> hovered_index) {
    for (size_t i = 0; i < action_lines.size(); ++i) {
      const bool selected = (i == selected_action_index);
      const bool hovered = hovered_index && *hovered_index == i;
      set_action_visual_state(action_lines[i], selected, hovered, !selected);
    }
  }

  void trigger_action(size_t index) {
    if (index == kRestartAction) {
      handle_restart();
      return;
    }
    if (index == kMainMenuAction) {
      handle_main_menu();
      return;
    }
    handle_toggle();
  }

  void update() override {
    update_pointer();

    const bool main_active = is_main_scene_active();
    if (!main_active) {
      set_pause_toggle_visible(false);
      apply_pause_toggle_hover(false);
      set_pause_menu_visible(false);
      return;
    }

    const bool paused = scene::is_current_scene_paused();
    const bool blocked = pause_controls_blocked();

    for (const auto& evt : input::events()) {
      if (evt.kind != engine::InputKind::KeyDown) continue;
      const int key = input::normalize_key_code(evt.key_code);
      if (key != 'P') continue;
      if (paused || !blocked) {
        handle_toggle();
        return;
      }
    }

    if (!paused) {
      set_pause_menu_visible(false);
      const bool show_toggle = !blocked;
      set_pause_toggle_visible(show_toggle);
      if (!show_toggle) {
        apply_pause_toggle_hover(false);
        return;
      }

      bool hovered_toggle = false;
      if (last_pointer) {
        const auto [min_bound, max_bound] = pause_toggle_bounds();
        hovered_toggle = point_in_bounds(*last_pointer, min_bound, max_bound);
      }
      apply_pause_toggle_hover(hovered_toggle);

      for (const auto& evt : input::events()) {
        if (evt.kind != engine::InputKind::PointerDown) continue;
        if (!hovered_toggle) continue;
        handle_toggle();
        return;
      }
      return;
    }

    set_pause_toggle_visible(false);
    apply_pause_toggle_hover(false);
    if (!pause_menu_open) {
      return;
    }

    std::optional<size_t> hovered_index;
    if (last_pointer) {
      for (size_t i = 0; i < action_lines.size(); ++i) {
        const auto [min_bound, max_bound] = action_bounds(action_lines[i]);
        if (!point_in_bounds(*last_pointer, min_bound, max_bound)) continue;
        hovered_index = i;
        break;
      }
    }
    if (hovered_index) {
      selected_action_index = *hovered_index;
    }
    apply_action_visuals(hovered_index);

    for (const auto& evt : input::events()) {
      if (evt.kind == engine::InputKind::PointerDown) {
        if (hovered_index) {
          trigger_action(*hovered_index);
          return;
        }
        continue;
      }
      if (evt.kind != engine::InputKind::KeyDown) continue;
      const int key = input::normalize_key_code(evt.key_code);
      if (is_arrow_up_key(key)) {
        move_selected_action(-1);
        apply_action_visuals(std::nullopt);
        continue;
      }
      if (is_arrow_down_key(key)) {
        move_selected_action(1);
        apply_action_visuals(std::nullopt);
        continue;
      }
      if (key == 'R') {
        trigger_action(kRestartAction);
        return;
      }
      if (key == 'M') {
        trigger_action(kMainMenuAction);
        return;
      }
      if (key == 27) {
        trigger_action(kResumeAction);
        return;
      }
      if (is_confirm_key(key)) {
        trigger_action(selected_action_index);
        return;
      }
    }
  }

  void handle_toggle() {
    if (!scene::is_current_scene_paused()) {
      camera_shake::reset();
      vfx::reset_wobble_offsets();
      selected_action_index = kResumeAction;
      if (overlay_hidden) overlay_hidden->show();
      set_pause_menu_visible(true);
      apply_action_visuals(std::nullopt);
      scene::toggle_pause();
      return;
    }

    set_pause_menu_visible(false);
    countdown::start("main", 3, []() {
      camera_shake::reset();
      vfx::reset_wobble_offsets();
      if (overlay_hidden) overlay_hidden->hide();
      if (auto* main = scene::get_scene("main")) {
        main->set_pause(false);
      }
    });
  }

  void handle_restart() {
    levels::restart_level();
    lives::reset_lives();
    player::reset_for_new_level();
    camera_shake::reset();
    vfx::reset_wobble_offsets();
    if (overlay_hidden) overlay_hidden->show();
    set_pause_menu_visible(false);
    if (auto* main = scene::get_scene("main")) {
      main->activate();
      main->set_pause(true);
    }
    countdown::start("main", 3, []() {
      camera_shake::reset();
      vfx::reset_wobble_offsets();
      if (overlay_hidden) overlay_hidden->hide();
      if (auto* main = scene::get_scene("main")) {
        main->set_pause(false);
      }
    });
  }

  void handle_main_menu() {
    camera_shake::reset();
    vfx::reset_wobble_offsets();
    countdown::cancel();
    set_pause_menu_visible(false);
    if (overlay_hidden) overlay_hidden->hide();
    if (auto* main = scene::get_scene("main")) {
      main->set_pause(true);
    }
    if (auto* menu_scene = scene::get_scene("menu")) {
      menu_scene->activate();
      menu_scene->set_pause(true);
    }
    menu::enter_main_menu_mode();
    menu::suppress_input_for_frames(2);
  }

  std::optional<glm::vec2> last_pointer;
  size_t selected_action_index = kResumeAction;
};

inline PauseMenuController pause_menu_controller{};

inline void init() {
  countdown::config.scale = config.countdown_scale;
  const glm::vec2 view_size{
      static_cast<float>(shrooms::screen::view_width),
      static_cast<float>(shrooms::screen::view_height),
  };

  overlay = arena::create<ecs::Entity>();
  auto* overlay_transform = arena::create<transform::NoRotationTransform>();
  overlay_transform->pos = glm::vec2{0.0f, 0.0f};
  overlay->add(overlay_transform);
  overlay->add(arena::create<layers::ConstLayer>(config.overlay_layer));
  overlay->add(arena::create<render_system::QuadRenderable>(
      view_size.x, view_size.y,
      engine::UIColor{config.overlay_color.x, config.overlay_color.y, config.overlay_color.z,
                      config.overlay_color.w}));
  overlay_hidden = arena::create<hidden::HiddenObject>();
  overlay->add(overlay_hidden);
  overlay_hidden->hide();
  overlay->add(arena::create<scene::SceneObject>("main"));

  menu_window = arena::create<ecs::Entity>();
  const glm::vec2 menu_size = shrooms::screen::scale_to_pixels(config.menu_scale);
  const glm::vec2 menu_center = shrooms::screen::norm_to_pixels(config.menu_position);
  auto* menu_transform = arena::create<transform::NoRotationTransform>();
  menu_transform->pos = shrooms::screen::center_to_top_left(menu_center, menu_size);
  menu_window->add(menu_transform);
  menu_window->add(arena::create<layers::ConstLayer>(config.menu_layer));
  const engine::TextureId menu_tex = engine::resources::register_texture("pause_menu");
  menu_window->add(arena::create<render_system::SpriteRenderable>(
      menu_tex, menu_size,
      engine::UIColor{config.menu_color.x, config.menu_color.y, config.menu_color.z,
                      config.menu_color.w}));
  menu_hidden = arena::create<hidden::HiddenObject>();
  menu_window->add(menu_hidden);
  menu_hidden->hide();
  menu_window->add(arena::create<scene::SceneObject>("main"));

  action_lines[kResumeAction] =
      make_action_line("Resume", config.menu_position + config.resume_offset, config.resume_scale);
  action_lines[kRestartAction] = make_action_line("Restart", config.menu_position + config.restart_offset,
                                                  config.restart_scale);
  action_lines[kMainMenuAction] =
      make_action_line("Main Menu", config.menu_position + config.main_menu_offset,
                       config.main_menu_scale);

  pause_toggle_button = arena::create<ecs::Entity>();
  pause_toggle_transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 pause_button_size = shrooms::screen::scale_to_pixels(config.pause_button_scale);
  const glm::vec2 pause_button_center = shrooms::screen::norm_to_pixels(config.pause_button_position);
  pause_toggle_transform->pos =
      shrooms::screen::center_to_top_left(pause_button_center, pause_button_size);
  pause_toggle_base_pos = pause_toggle_transform->pos;
  pause_toggle_base_size = pause_button_size;
  pause_toggle_button->add(pause_toggle_transform);
  pause_toggle_button->add(arena::create<layers::ConstLayer>(config.pause_button_layer));
  pause_toggle_quad = arena::create<render_system::QuadRenderable>(
      pause_button_size.x, pause_button_size.y, pause_toggle_base_color);
  pause_toggle_button->add(pause_toggle_quad);
  pause_toggle_button_hidden = arena::create<hidden::HiddenObject>();
  pause_toggle_button->add(pause_toggle_button_hidden);
  pause_toggle_button_hidden->hide();
  pause_toggle_button->add(arena::create<scene::SceneObject>("main"));

  pause_toggle_icon = arena::create<ecs::Entity>();
  auto* pause_toggle_icon_transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 pause_icon_size = shrooms::screen::scale_to_pixels(config.pause_icon_scale);
  pause_toggle_icon_transform->pos =
      shrooms::screen::center_to_top_left(pause_button_center, pause_icon_size);
  pause_toggle_icon->add(pause_toggle_icon_transform);
  pause_toggle_icon->add(arena::create<layers::ConstLayer>(config.pause_button_layer + 1));
  pause_toggle_icon_sprite = arena::create<render_system::SpriteRenderable>(
      menu_tex, pause_icon_size, engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f});
  pause_toggle_icon->add(pause_toggle_icon_sprite);
  pause_toggle_icon_hidden = arena::create<hidden::HiddenObject>();
  pause_toggle_icon->add(pause_toggle_icon_hidden);
  pause_toggle_icon_hidden->hide();
  pause_toggle_icon->add(arena::create<scene::SceneObject>("main"));
}

}  // namespace pause_menu
