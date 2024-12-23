#pragma once
#include "../definitions/components/configurable_object.hpp"
#include "../utils/logger.hpp"
#include "../ecs/ecs.hpp"
namespace logger {
    bool enable_text_system_logging_ = false;
    inline config::BoolParameter enable_text_system_logging{"Enable text system logging", &enable_text_system_logging_};

    bool enable_collision_system_logging_ = false;
    inline config::BoolParameter enable_collision_system_logging{"Enable collision system logging", &enable_collision_system_logging_};

    bool enable_scene_system_logging_ = false;
    inline config::BoolParameter enable_scene_system_logging{"Enable scene system logging", &enable_scene_system_logging_};
 
    bool enable_gpu_program_system_logging_ = false;
    inline config::BoolParameter enable_gpu_program_system_logging{"Enable gpu program system logging", &enable_gpu_program_system_logging_};
 
    inline config::BoolParameter enable_ecs_logging{"Enable ecs logging", &enable_ecs_logging_};
}