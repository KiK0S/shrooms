#pragma once
#include <string>
#include <vector>
#include <variant>

namespace debug {

class ObservableObject;
static std::vector<ObservableObject*> observables;

class ObservableObject {
public:
    ObservableObject() {
        observables.push_back(this);
    }
    virtual ~ObservableObject() {
        auto it = std::find(observables.begin(), observables.end(), this);
        if (it != observables.end()) {
            observables.erase(it);
        }
    }

    virtual void register_imgui() = 0;
};
} // namespace debug 