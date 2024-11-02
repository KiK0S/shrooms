#pragma once
#include "../definitions/components/text_object.hpp"
#include "../definitions/systems/scene_system.hpp"
#include "../declarations/text_system.hpp"
#include "../declarations/color_system.hpp"

namespace scoreboard {

text::TextObject* score_text;
scene::SceneObject score_scene("main");
ecs::Entity score_display;
layers::ConstLayer score_layer(5);
transform::NoRotationTransform score_transform({0.6f, 0.7f}, {0.9f, 0.95f});
int score = 0;

void update_score(int new_score) {
    score = new_score;
    score_display.detach(score_text);
    auto geom = score_display.get_checked<text::TextGeometry>();
    score_display.detach(geom);
    score_text = arena::create<text::TextObject>("Score: " + std::to_string(new_score));
    score_display.add(score_text);
    text::text_loader.init(score_text);
}

void init() {
    score_text = arena::create<text::TextObject>("Score: 0");

    std::cout << "init scoreboard\n";
    score_display.add(score_text)
                .add(&score_scene)
                .add(&score_layer)
                .add(&shaders::model_matrix)
	            .add(arena::create<shaders::ProgramArgumentObject>(&shaders::static_object_program))
                .add(&score_transform)
                .add(&color::white)
                .add(text::text_texture.get())
                .bind();
    text::text_loader.init(score_text);
}

init::CallbackOnStart scoreboard_init(&init, 6);

} // namespace scoreboard
