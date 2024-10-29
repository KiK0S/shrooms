#pragma once
#include <vector>
#include <algorithm>
#include "../components/dynamic_object.hpp"
#include "../components/init_object.hpp"

namespace dynamic {
struct cmp {
	bool operator()(DynamicObject* a, DynamicObject* b) const {
        return a->get_priority() < b->get_priority();
    }
};


}

