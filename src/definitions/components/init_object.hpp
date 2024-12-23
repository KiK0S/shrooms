#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include "../../ecs/component.hpp"

namespace init {

struct UnInitializedObject;

COMPONENT_VECTOR(UnInitializedObject, initializable);

struct cmp {
	bool operator()(UnInitializedObject* a, UnInitializedObject* b) const;
};

struct UnInitializedObject: public ecs::Component {
	UnInitializedObject(int priority = 0): ecs::Component(), priority(priority) {
		initializable.push_back(this);
	}
	virtual ~UnInitializedObject() {
		Component::component_count--;
	}
	void bind(ecs::Entity*) {}
	virtual void init() = 0;
	int get_priority() {
		return priority;
	}
	int priority = 0;
	DETACH_VECTOR(UnInitializedObject, initializable)
};


void init() {
	std::sort(initializable.begin(), initializable.end(), cmp());
	for (auto x : initializable) {
		std::cerr << "init " << typeid(*x).name() << '\n';
		x->init();
	}
}

struct CallbackOnStart: public UnInitializedObject {
	CallbackOnStart(std::function<void()>&& callback, int priority = 0): UnInitializedObject(priority), callback(std::move(callback)) {	}
	virtual ~CallbackOnStart() {
		Component::component_count--;
	}
	void init() {
		callback();
	}
	std::function<void()> callback;
};



bool cmp::operator()(UnInitializedObject* a, UnInitializedObject* b) const {
	return a->get_priority() < b->get_priority();
}

}
