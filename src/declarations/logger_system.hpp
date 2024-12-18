#pragma once
#include "../definitions/components/configurable_object.hpp"

namespace logger {
    bool enable_text_system_logging_ = false;
    inline config::BoolParameter enable_text_system_logging{"Enable text system logging", &enable_text_system_logging_};

    bool enable_collision_system_logging_ = false;
    inline config::BoolParameter enable_collision_system_logging{"Enable collision system logging", &enable_collision_system_logging_};
}
