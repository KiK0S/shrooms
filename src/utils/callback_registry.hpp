#pragma once
#include <map>
#include <functional>
#include <string>
#include <iostream>

template<typename CallbackType>
class CallbackRegistry {
private:
    CallbackRegistry() = default;

    static std::map<std::string, CallbackType>& get_callbacks() {
        static std::map<std::string, CallbackType> callbacks;
        return callbacks;
    }

public:
    class Registrar {
    public:
        Registrar(const std::string& name, CallbackType callback) {
            CallbackRegistry<CallbackType>::register_callback(name, callback);
        }
    };

    static void register_callback(const std::string& name, CallbackType callback) {
        get_callbacks()[name] = callback;
    }

    static CallbackType get_callback(const std::string& name) {
        auto& callbacks = get_callbacks();
        auto it = callbacks.find(name);
        if (it != callbacks.end()) {
            return it->second;
        }
        std::cerr << "Warning: No callback registered for name: " << name << std::endl;
        return nullptr;
    }
}; 