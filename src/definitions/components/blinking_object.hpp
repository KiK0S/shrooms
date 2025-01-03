#pragma once

#include "../../ecs/ecs.hpp"
#include "../components/hidden_object.hpp"
#include <chrono>
#include <vector>

namespace blinking {

struct BlinkingObject;
COMPONENT_VECTOR(BlinkingObject, blinking_objects);

struct BlinkingObject : public ecs::Component {
    BlinkingObject() : ecs::Component() {
        blinking_objects.push_back(this);
    }
    
    DETACH_VECTOR(BlinkingObject, blinking_objects)
    ~BlinkingObject() {
        ecs::Component::component_count--;
    }

    void activate(float period_seconds, int blink_count) {
        LOG_IF(logger::enable_blinking_system_logging, "activating blinking object");
        if (is_active) return;

        this->period = period_seconds;
        this->remaining_blinks = blink_count * 2; // *2 because each blink is show+hide
        this->is_active = true;
        this->last_switch = std::chrono::steady_clock::now();
    }

    bool is_blinking() const { return is_active; }
    float get_period() const { return period; }
    int get_remaining_blinks() const { return remaining_blinks; }
    const auto& get_last_switch() const { return last_switch; }
    
    void set_active(bool active) { is_active = active; }
    void set_remaining_blinks(int blinks) { remaining_blinks = blinks; }
    void set_last_switch(std::chrono::steady_clock::time_point time) { last_switch = time; }

private:
    float period = 0.0f;
    int remaining_blinks = 0;
    bool is_active = false;
    std::chrono::steady_clock::time_point last_switch;
};

} // namespace blinking 