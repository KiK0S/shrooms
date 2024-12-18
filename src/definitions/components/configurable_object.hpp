#pragma once
#include "../../ecs/component.hpp"
#include <string>
#include <vector>
#include "imgui.h"

namespace config {

struct ConfigurableObject;
COMPONENT_VECTOR(ConfigurableObject, configurables);

struct ConfigurableObject : public ecs::Component {
    ConfigurableObject() : ecs::Component() {
        configurables.push_back(this);
    }
    virtual ~ConfigurableObject() {}
    virtual void register_imgui() = 0;
	DETACH_VECTOR(ConfigurableObject, configurables)
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

struct BoolParameter : public ConfigurableObject {
    BoolParameter(const std::string& name, bool* value)
        : name(name), value(value) {}

    void register_imgui() override {
        ImGui::Checkbox(name.c_str(), value);
    }

    operator bool() const {
        return *value;
    }

    std::string name;
    bool* value;
};


} // namespace config 