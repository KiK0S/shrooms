#pragma once

#include "systems/collision/collider_object.hpp"
#include "utils/callback_registry.hpp"

#include "level_manager.hpp"
#include "player.hpp"
#include "game_audio.hpp"
#include "engine/resource_ids.h"
#include "systems/render/sprite_system.hpp"
#include "vfx.hpp"

namespace shrooms {

inline void mushroom_fall_handler(ecs::Entity*, collision::ColliderObject* collider) {
  if (!collider) return;
  auto* entity = collider->get_entity();
  if (!entity || entity->is_pending_deletion()) return;
  if (vfx::is_catch_animating(entity)) return;
  if (entity->get<player::CarriedMarker>()) return;
  auto* sprite = entity->get<render_system::SpriteRenderable>();
  const std::string type = sprite ? engine::resources::texture_name(sprite->texture_id) : "";
  levels::on_mushroom_missed(type, entity);
  shrooms::audio::play_mushroom_fall();
  entity->mark_deleted();
}

inline collision::TriggerCallbackRegistry::Registrar mushroom_fall_registrar(
    "mushroom_fall_handler", mushroom_fall_handler);

}  // namespace shrooms
