#pragma once
#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace ecs {
class Entity;  // Forward declaration
class Component;

// Global registry of all component vectors
inline std::map<std::string, void*> component_registries;
inline std::map<std::string, bool> container_is_vector;

void reg_component(Entity* e, Component* c);

#define COMPONENT_VECTOR(T, name) \
    inline std::vector<T*> name; \
    struct name##_registrar { \
        name##_registrar() { \
            ecs::component_registries[#T] = (std::vector<ecs::Component*>*)&name; \
            ecs::container_is_vector[#T] = true; \
        } \
    }; \
    inline name##_registrar name##_reg;

#define COMPONENT_MAP(T, name) \
    inline std::map<std::string, std::vector<T*>> name; \
    struct name##_registrar { \
        name##_registrar() { \
            ecs::component_registries[#T] = (std::map<std::string, std::vector<ecs::Component*>>*)&name; \
            ecs::container_is_vector[#T] = false; \
        } \
    }; \
    inline name##_registrar name##_reg;

#define DETACH_VECTOR(T, Name) \
    virtual void detach() override { \
        if (entity == nullptr || global) return; \
        auto vec = static_cast<std::vector<T*>*>(ecs::component_registries[#T]); \
        vec->erase(std::remove_if(vec->begin(), vec->end(), [this](T* c) { return c == this; }), vec->end()); \
        this->detach_specific(); \
        LOG_IF(enable_ecs_logging_, "Detached " << typeid(*this).name() << " from " << #T); \
        this->~T(); \
    }

#define DETACH_MAP(T, Name) \
    virtual void detach() override { \
        if (entity == nullptr || global) return; \
        auto map = static_cast<std::map<std::string, std::vector<T*>>*>(ecs::component_registries[#T]); \
        for (auto& [_, vec] : *map) \
            vec.erase(std::remove_if(vec.begin(), vec.end(), [this](T* c) { return c == this; }), vec.end()); \
        LOG_IF(enable_ecs_logging_, "Detached " << typeid(*this).name() << " from " << #T); \
        this->detach_specific(); \
        this->~T(); \
    }

class Component {
public:
    Component() {
        LOG_IF(enable_ecs_logging_, "created " << typeid(*this).name());
        component_count++;
    }
    virtual ~Component() {
        LOG_IF(enable_ecs_logging_, "deleted " << typeid(*this).name());
        component_count--;
    }
    virtual void bind(Entity* entity) {
        this->entity = entity;
        reg_component(entity, this);
    }
    virtual ecs::Entity* get_entity() {
        return entity;
    }
    virtual void detach_specific() {}
    virtual void detach() {
        if (entity != nullptr && !global) {
            this->~Component();
        }
    }
    Entity* entity = nullptr;
    bool global = false;
    static size_t get_component_count() { return component_count; }

protected:
    static inline size_t component_count = 0;
};
}
