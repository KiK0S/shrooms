#pragma once
#include "../definitions/components/geometry_object.hpp"
#include <vector>
#include <glm/vec2.hpp>

namespace geometry {

struct Circle : public geometry::GeometryObject {
    Circle(): geometry::GeometryObject() {}

    std::vector<glm::vec2> get_pos() {
        return {
            // First triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{1.0f, 0.0f},
            glm::vec2{0.92388f, 0.382683f},

            // Second triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.92388f, 0.382683f},
            glm::vec2{0.707107f, 0.707107f},

            // Third triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.707107f, 0.707107f},
            glm::vec2{0.382683f, 0.92388f},

            // Fourth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.382683f, 0.92388f},
            glm::vec2{0.0f, 1.0f},

            // Fifth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.0f, 1.0f},
            glm::vec2{-0.382683f, 0.92388f},

            // Sixth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{-0.382683f, 0.92388f},
            glm::vec2{-0.707107f, 0.707107f},

            // Seventh triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{-0.707107f, 0.707107f},
            glm::vec2{-0.92388f, 0.382683f},

            // Eighth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{-0.92388f, 0.382683f},
            glm::vec2{-1.0f, 0.0f},

            // Ninth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{-1.0f, 0.0f},
            glm::vec2{-0.92388f, -0.382683f},

            // Tenth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{-0.92388f, -0.382683f},
            glm::vec2{-0.707107f, -0.707107f},

            // Eleventh triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{-0.707107f, -0.707107f},
            glm::vec2{-0.382683f, -0.92388f},

            // Twelfth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{-0.382683f, -0.92388f},
            glm::vec2{0.0f, -1.0f},

            // Thirteenth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.0f, -1.0f},
            glm::vec2{0.382683f, -0.92388f},

            // Fourteenth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.382683f, -0.92388f},
            glm::vec2{0.707107f, -0.707107f},

            // Fifteenth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.707107f, -0.707107f},
            glm::vec2{0.92388f, -0.382683f},

            // Sixteenth triangle
            glm::vec2{0.0f, 0.0f},
            glm::vec2{0.92388f, -0.382683f},
            glm::vec2{1.0f, 0.0f}
        };
    }

    std::vector<glm::vec2> get_uv() {
        std::vector<glm::vec2> uv_coords(48, glm::vec2{0.0f, 0.0f});
        return uv_coords;
    }

    std::string get_name() const {
        return "circle";
    }

    int get_size() {
        return 48;
    }
};

Circle circle;

}
