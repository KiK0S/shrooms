#pragma once

#include "../definitions/components/dynamic_object.hpp"
#include "../definitions/components/layered_object.hpp"
#include "../definitions/components/shader_object.hpp"
#include "../definitions/components/text_object.hpp"
#include "../definitions/components/textured_object.hpp"
#include "../definitions/components/transform_object.hpp"
#include "../definitions/systems/input_system.hpp"
#include "../declarations/color_system.hpp"
#include "../declarations/shader_system.hpp"
#include "../declarations/text_system.hpp"
#include "../declarations/scene_system.hpp"
#include "../geometry/quad.hpp"
#include "../utils/arena.hpp"
#include "level_manager.hpp"
#include <glm/glm.hpp>
#include "player.hpp"
#include "lives.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <optional>
#include <string>

namespace menu {

constexpr size_t MAX_LEVEL_LINES = 10;

struct TextLine {
    ecs::Entity entity;
    transform::NoRotationTransform transform;
    text::TextObject* text_object = nullptr;
    float target_height = 0.09f;
    float min_width = 0.35f;
    float max_width = 1.6f;
};

TextLine status_line;
TextLine instruction_line;
TextLine credits_line;
std::array<TextLine, MAX_LEVEL_LINES> level_lines;
size_t active_level_lines = 0;

ecs::Entity character_entity;
transform::NoRotationTransform character_transform;

struct MenuController : public dynamic::DynamicObject {
    MenuController(): dynamic::DynamicObject() {}

    void update() override;

    std::array<bool, MAX_LEVEL_LINES> previous_state{};
    size_t cached_level_count = std::numeric_limits<size_t>::max();
    size_t cached_last_played = std::numeric_limits<size_t>::max();
    std::string cached_status;
    bool completion_acknowledged = false;
} controller;

void update_text(TextLine& line, const std::string& value) {
    if (line.text_object) {
        line.entity.detach(line.text_object);
    }
    if (auto* geom = line.entity.get<text::TextGeometry>()) {
        line.entity.detach(geom);
    }
    line.text_object = arena::create<text::TextObject>(value);
    line.entity.add(line.text_object);
    text::text_loader.init(line.text_object);
    if (auto* geom = line.entity.get<text::TextGeometry>()) {
        float logical_height = std::max(geom->logical_height, 0.000000000001f);
        float aspect = geom->logical_width / logical_height;
        if (aspect <= 0.0f) {
            aspect = 1.0f;
        }
        float height = line.target_height;
        float width = height * aspect;
        if (line.max_width > 0.0f && width > line.max_width) {
            float scale_factor = line.max_width / std::max(width, 0.0001f);
            width = line.max_width;
            height *= scale_factor;
        }
        if (line.min_width > 0.0f && width < line.min_width) {
            width = line.min_width;
        }
        line.transform.scale_ = glm::vec2(width * 0.5f, height * 0.5f);
    }
}

size_t available_levels() {
    return std::min(MAX_LEVEL_LINES, levels::parsed_levels.size());
}

std::string format_level_line(size_t index) {
    if (index >= levels::parsed_levels.size()) {
        return "";
    }
    const auto& definition = levels::parsed_levels[index];
    std::string label = std::to_string(index + 1) + ". " + definition.id;
    if (levels::last_played_level_index == index && !levels::level_finished) {
        label += "  [in progress]";
    } else if (levels::last_played_level_index == index && levels::level_finished) {
        label += "  [last]";
    }
    return label;
}

void refresh_status_line() {
    update_text(status_line, "Last game: " + levels::last_game_status);
}

void refresh_instruction_line() {
    size_t count = available_levels();
    if (count == 0) {
        update_text(instruction_line, "No levels available yet. Use keyboard to navigate; tap/click is unavailable.");
        return;
    }
    std::string suffix = count == MAX_LEVEL_LINES && levels::parsed_levels.size() > MAX_LEVEL_LINES
        ? " (first 10 shown)"
        : "";
    if (count == 1) {
        update_text(instruction_line, "Use number key 1 or tap/click the entry to start the level" + suffix + ".");
    } else {
        update_text(instruction_line, "Use number keys 1-" + std::to_string(count) + " or tap/click a level to start" + suffix + ".");
    }
}

void refresh_level_lines() {
    active_level_lines = available_levels();
    for (size_t i = 0; i < MAX_LEVEL_LINES; ++i) {
        if (i < active_level_lines) {
            update_text(level_lines[i], format_level_line(i));
        } else {
            update_text(level_lines[i], "");
        }
    }
}

std::optional<size_t> level_index_at_point(const glm::vec2& point) {
    size_t count = active_level_lines;
    for (size_t i = 0; i < count; ++i) {
        const auto& line = level_lines[i];
        const glm::vec2 center = line.transform.pos;
        const glm::vec2 half_size = line.transform.scale_;
        if (point.x < center.x - half_size.x || point.x > center.x + half_size.x) {
            continue;
        }
        if (point.y < center.y - half_size.y || point.y > center.y + half_size.y) {
            continue;
        }
        return i;
    }
    return std::nullopt;
}

std::optional<glm::vec2> normalized_from_mouse(const SDL_MouseButtonEvent& button) {
    SDL_Window* window = SDL_GetWindowFromID(button.windowID);
    if (!window) {
        return std::nullopt;
    }
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return std::nullopt;
    }
    float norm_x = (static_cast<float>(button.x) / static_cast<float>(width)) * 2.0f - 1.0f;
    float norm_y = 1.0f - (static_cast<float>(button.y) / static_cast<float>(height)) * 2.0f;
    return glm::vec2(norm_x, norm_y);
}

glm::vec2 normalized_from_touch(const SDL_TouchFingerEvent& finger) {
    float norm_x = static_cast<float>(finger.x) * 2.0f - 1.0f;
    float norm_y = 1.0f - static_cast<float>(finger.y) * 2.0f;
    return glm::vec2(norm_x, norm_y);
}

void handle_level_selection(size_t index) {
    if (index >= levels::parsed_levels.size()) {
        return;
    }
    lives::reset_lives();
    levels::start_level(index);
    player::reset_for_new_level();
    scene::main.activate();
    scene::main.set_pause(false);
    scene::menu.set_pause(true);
    controller.cached_last_played = levels::last_played_level_index;
    refresh_level_lines();
    controller.cached_status = levels::last_game_status;
    refresh_status_line();
}

void MenuController::update() {
    size_t current_levels = available_levels();
    if (cached_level_count != current_levels) {
        cached_level_count = current_levels;
        refresh_level_lines();
        refresh_instruction_line();
    } else if (cached_last_played != levels::last_played_level_index) {
        cached_last_played = levels::last_played_level_index;
        refresh_level_lines();
    }

    if (cached_status != levels::last_game_status) {
        cached_status = levels::last_game_status;
        refresh_status_line();
    }

    if (levels::level_finished) {
        if (!completion_acknowledged && !scene::menu.is_active) {
            scene::menu.activate();
            scene::menu.set_pause(true);
            scene::main.set_pause(true);
            player::reset_for_new_level();
            completion_acknowledged = true;
        }
    } else {
        completion_acknowledged = false;
    }

    if (!scene::menu.is_active) {
        return;
    }

    if (!scene::menu.is_paused_state()) {
        scene::menu.set_pause(true);
    }

    constexpr std::array<std::pair<SDL_Scancode, size_t>, MAX_LEVEL_LINES> key_map = {{
        {SDL_SCANCODE_1, 0},
        {SDL_SCANCODE_2, 1},
        {SDL_SCANCODE_3, 2},
        {SDL_SCANCODE_4, 3},
        {SDL_SCANCODE_5, 4},
        {SDL_SCANCODE_6, 5},
        {SDL_SCANCODE_7, 6},
        {SDL_SCANCODE_8, 7},
        {SDL_SCANCODE_9, 8},
        {SDL_SCANCODE_0, 9},
    }};

    for (size_t i = 0; i < MAX_LEVEL_LINES; ++i) {
        bool pressed = input::get_button_state(key_map[i].first);
        if (pressed && !previous_state[i] && key_map[i].second < current_levels) {
            handle_level_selection(key_map[i].second);
        }
        previous_state[i] = pressed;
    }
}

struct MenuPointerController : public input::ControllableObject {
    MenuPointerController() : input::ControllableObject() {}

    void handle_user_action(SDL_Event event) override {
        if (!scene::menu.is_active) {
            return;
        }
        switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (event.button.button != SDL_BUTTON_LEFT) {
                return;
            }
            auto normalized = normalized_from_mouse(event.button);
            if (!normalized) {
                return;
            }
            try_select(*normalized);
            break;
        }
        case SDL_FINGERDOWN: {
            glm::vec2 normalized = normalized_from_touch(event.tfinger);
            try_select(normalized);
            break;
        }
        default:
            break;
        }
    }

  private:
    void try_select(const glm::vec2& point) const {
        auto index = level_index_at_point(point);
        if (!index) {
            return;
        }
        handle_level_selection(*index);
    }
};
MenuPointerController pointer_controller;

void init_text_line(TextLine& line, glm::vec2 position, int layer, float target_height, float min_width, float max_width) {
    auto program = arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program);
    auto model = arena::create<shaders::ModelMatrix>();
    auto layer_comp = arena::create<layers::ConstLayer>(layer);

    line.target_height = target_height;
    line.min_width = min_width;
    line.max_width = max_width;
    line.transform.scale_ = glm::vec2(target_height * 0.5f, target_height * 0.5f);
    line.transform.translate(position);

    line.entity.add(program)
              .add(&line.transform)
              .add(model)
              .add(layer_comp)
              .add(arena::create<scene::SceneObject>("menu"))
              .add(&color::white)
              .add(text::text_texture)
              .bind();
}

void init_level_line(size_t index, glm::vec2 base_position, float spacing, int layer, float target_height, float min_width, float max_width) {
    glm::vec2 position = base_position - glm::vec2(0.0f, spacing * static_cast<float>(index));
    init_text_line(level_lines[index], position, layer, target_height, min_width, max_width);
    update_text(level_lines[index], "");
}

void init() {
    character_transform.scale(glm::vec2(0.4f, 0.4f));
    character_transform.translate(glm::vec2(0.65f, -0.2f));
    auto character_layer = arena::create<layers::ConstLayer>(3);
    auto character_program = arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program);
    auto character_model = arena::create<shaders::ModelMatrix>();
    auto character_texture = arena::create<texture::OneTextureObject>("witch");
    character_entity.add(&geometry::quad)
                    .add(character_layer)
                    .add(character_program)
                    .add(&character_transform)
                    .add(character_model)
                    .add(character_texture)
                    .add(arena::create<scene::SceneObject>("menu"))
                    .bind();

    init_text_line(status_line, glm::vec2(0.0f, 0.65f), 6, 0.12f, 0.9f, 1.8f);
    init_text_line(instruction_line, glm::vec2(0.0f, 0.48f), 6, 0.11f, 0.1f, 1.8f);
    init_text_line(credits_line, glm::vec2(0.0f, -0.85f), 6, 0.095f, 0.9f, 1.8f);

    update_text(credits_line, "Game by KiK0S, art by deadmarla.");

    glm::vec2 base_position = glm::vec2(-0.4f, 0.25f);
    float spacing = 0.16f;
    for (size_t i = 0; i < MAX_LEVEL_LINES; ++i) {
        init_level_line(i, base_position, spacing, 6, 0.08f, 0.1f, 1.0f);
    }

    refresh_status_line();
    refresh_instruction_line();
    refresh_level_lines();
}

init::CallbackOnStart menu_init(&init, 8);

} // namespace menu
