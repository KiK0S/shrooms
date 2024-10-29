#pragma once
#include "geometry/circle.hpp"
#include "geometry/curve.hpp"
#include "geometry/polygon.hpp"
#include "geometry/quad.hpp"
#include "geometry/triangle.hpp"

#include "../components/transform_object.hpp"
#include "../components/geometry_object.hpp"
#include "../../ecs/ecs.hpp"
#include "glm/glm/vec2.hpp"

namespace geometry {

class BoundingBox {
public:
    BoundingBox(const glm::vec2& min, const glm::vec2& max) : min_(min), max_(max) {}

    glm::vec2 get_min() const { return min_; }
    glm::vec2 get_max() const { return max_; }

    bool intersects(const BoundingBox& other) const {
        return (min_.x <= other.max_.x && max_.x >= other.min_.x) &&
               (min_.y <= other.max_.y && max_.y >= other.min_.y);
    }

private:
    glm::vec2 min_;
    glm::vec2 max_;
};

BoundingBox get_bounding_box(ecs::Entity* entity) {
    auto geometry = entity->get<GeometryObject>();
    auto transform = entity->get<transform::TransformObject>();
    
    if (!geometry || !transform) {
        return BoundingBox(glm::vec2(0.0f), glm::vec2(0.0f));
    }
    
    const auto& positions = geometry->get_pos();
    
    if (positions.empty()) {
        return BoundingBox(glm::vec2(0.0f), glm::vec2(0.0f));
    }

    glm::vec2 min = transform->transform_point(positions[0]);
    glm::vec2 max = min;

    for (const auto& pos : positions) {
        glm::vec2 transformed = transform->transform_point(pos);
        min.x = std::min(min.x, transformed.x);
        min.y = std::min(min.y, transformed.y);
        max.x = std::max(max.x, transformed.x);
        max.y = std::max(max.y, transformed.y);
    }

    return BoundingBox(min, max);
}

} // namespace geometry

