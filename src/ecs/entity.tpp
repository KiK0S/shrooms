#pragma once


template <typename T>
std::vector<T*> Entity::get_all() const {
    std::vector<T*> res;
    for (const auto comp : components) {
        if (auto casted_ptr = dynamic_cast<T*>(comp)) {
            res.push_back(casted_ptr);
        }
    }
    for (const auto comp : pre_bind_components) {
        if (auto casted_ptr = dynamic_cast<T*>(comp)) {
            res.push_back(casted_ptr);
        }
    }
    return res;
}

template <typename T>
T* Entity::get() const {
    for (const auto comp : components) {
        if (auto casted_ptr = dynamic_cast<T*>(comp)) {
            return casted_ptr;
        }
    }

    for (const auto comp : pre_bind_components) {
        if (auto casted_ptr = dynamic_cast<T*>(comp)) {
            return casted_ptr;
        }
    }
    return nullptr;
}

template <typename T>
T* Entity::get_checked() const {
    auto res = get<T>();
    if (!res) throw std::runtime_error("Component not found: " + std::string(typeid(T).name()));
    return res;
}
