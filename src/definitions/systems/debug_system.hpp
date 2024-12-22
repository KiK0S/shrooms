#pragma once
#include "../components/observable_object.hpp"
#include "../components/dynamic_object.hpp"
#include "../../ecs/entity.hpp"
#include "../../ecs/component.hpp"

namespace debug_system {

struct DebugSystem : public debug::ObservableObject, public dynamic::DynamicObject {
    DebugSystem() : DynamicObject(0) {}
    virtual ~DebugSystem() {
        Component::component_count--;
    }
    void update() {
        update_fps();
    }

    void register_imgui() override {
        #ifndef __EMSCRIPTEN__
        if (ImGui::CollapsingHeader("System Stats")) {
            ImGui::Text("Entity Count: %zu", ecs::Entity::get_entity_count());
            ImGui::Text("Component Count: %zu", ecs::Component::get_component_count());
            ImGui::Text("FPS: %.1f", fps);
            
            if (ImGui::TreeNode("Component Registry")) {
                for (const auto& [name, registry] : ecs::component_registries) {
                    bool is_vector = ecs::container_is_vector[name];
                    ImGui::Text("%s (%ld)", name.c_str(), is_vector ? ((std::vector<ecs::Component*>*)registry)->size() : 
                        ((std::map<std::string, std::vector<ecs::Component*>>*)registry)->size());
                }
                ImGui::TreePop();
            }
        }
        #endif
    }

private:
    float fps = 0.0f;
    float last_frame_time = 0.0f;

    void update_fps() {
        float current_time = ImGui::GetTime();
        float delta = current_time - last_frame_time;
        last_frame_time = current_time;
        
        const float smoothing = 0.1f;
        float current_fps = 1.0f / (delta > 0.0f ? delta : 0.0001f);
        fps = fps * (1.0f - smoothing) + current_fps * smoothing;
    }
};

} // namespace debug_system 