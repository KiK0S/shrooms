#pragma once

#include "../components/dynamic_object.hpp"
#include "../components/blinking_object.hpp"
#include "../../ecs/ecs.hpp"
#include <chrono>

namespace blinking {

struct BlinkingSystem : public dynamic::DynamicObject {
    BlinkingSystem() : dynamic::DynamicObject(50) {} // Priority 50 - adjust as needed

    void update() override {
        for (auto blink_comp : blinking_objects) {
            if (!blink_comp->is_blinking()) continue;
            LOG_IF(logger::enable_blinking_system_logging, "object is blinking");
            auto current_time = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(current_time - blink_comp->get_last_switch()).count();

            if (elapsed >= blink_comp->get_period()) {
                auto entity = blink_comp->get_entity();
                auto hidden = entity->get<hidden::HiddenObject>();
                if (hidden) {
                    hidden->set_visible(!hidden->is_visible());
                    blink_comp->set_remaining_blinks(blink_comp->get_remaining_blinks() - 1);
                    blink_comp->set_last_switch(current_time);

                    if (blink_comp->get_remaining_blinks() <= 0) {
                        blink_comp->set_active(false);
                        hidden->set_visible(true); // Ensure visible when done
                    }
                }
            }
        }
    }
};

} // namespace blinking 