#pragma once

#include "ecs/driver.hpp"

namespace engine::shrooms {

bool is_gameplay_active();
int action_key_code(int action_index);
void set_page_active(bool active);
void set_touchscreen_enabled(bool enabled);
void set_mobile_layout(bool enabled);

class ShroomsLogic : public ecs::EcsLogic {
 public:
  ShroomsLogic(int view_width, int view_height);

 protected:
  void on_init() override;
  void after_tick(const engine::AppContext&, std::span<const engine::InputEvent>,
                  engine::Frame&) override;

 private:
  int view_width_ = 0;
  int view_height_ = 0;
};

}  // namespace engine::shrooms
