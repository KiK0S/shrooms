#pragma once
#include "../../ecs/component.hpp"
#include <string>
#include <vector>
#include "imgui.h"

namespace config {

struct ConfigurableObject;
inline std::vector<ConfigurableObject*> configurables;

struct ConfigurableObject : public ecs::Component {
    ConfigurableObject() : ecs::Component() {
        configurables.push_back(this);
    }
    virtual ~ConfigurableObject() {}
    virtual void register_imgui() = 0;
};

struct FloatParameter : public ConfigurableObject {
    FloatParameter(const std::string& name, float* value, float min_val, float max_val) 
        : name(name), value(value), min_val(min_val), max_val(max_val) {}

    void register_imgui() override {
        ImGui::SliderFloat(name.c_str(), value, min_val, max_val);
    }

    std::string name;
    float* value;
    float min_val;
    float max_val;
};

} // namespace config 