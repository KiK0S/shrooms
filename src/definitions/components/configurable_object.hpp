#pragma once
#include "../../ecs/component.hpp"
#include <string>
#include <vector>
#include "imgui.h"
#include <glm/glm.hpp>

namespace config {

struct ConfigurableObject;
COMPONENT_VECTOR(ConfigurableObject, configurables);

struct ConfigurableObject : public ecs::Component {
    ConfigurableObject() : ecs::Component() {
        configurables.push_back(this);
    }
    virtual ~ConfigurableObject() {
        Component::component_count--;
    }
    virtual void register_imgui() = 0;
	DETACH_VECTOR(ConfigurableObject, configurables)
};

struct FloatParameter : public ConfigurableObject {
    FloatParameter(const std::string& name, float* value, float min_val, float max_val) 
        : name(name), value(value), min_val(min_val), max_val(max_val) {}

    virtual ~FloatParameter() {
        Component::component_count--;
    }

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
    virtual ~BoolParameter() {
        Component::component_count--;
    }
    void register_imgui() override {
        ImGui::Checkbox(name.c_str(), value);
    }

    operator bool() const {
        return *value;
    }

    std::string name;
    bool* value;
};

class Vec2Parameter : public ConfigurableObject {
private:
    glm::vec2* value;
    glm::vec2 min_value;
    glm::vec2 max_value;
    std::string name;

public:
    Vec2Parameter(const std::string& param_name, glm::vec2* param_value, 
                 glm::vec2 min_val, glm::vec2 max_val)
        : value(param_value)
        , min_value(min_val)
        , max_value(max_val)
        , name(param_name) {
    }

    void register_imgui() override {
        ImGui::DragFloat2(name.c_str(), &((*value)[0]), 0.01f, 
                         min_value.x, max_value.x, "%.3f");
    }
};

struct ButtonParameter : public ConfigurableObject {
    ButtonParameter(const std::string& name, std::function<void()> callback) : name(name), callback(callback) {}
    void register_imgui() override {
        if (ImGui::Button(name.c_str())) {
            callback();
        }
    }
    std::string name;
    std::function<void()> callback;
};

} // namespace config 