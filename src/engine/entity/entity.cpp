#include "entity.h"

#include <algorithm>
#include <format>

#include "engine/components/lua_script_component.h"
#include "engine/components/physics/rigidbody_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/systems/entity_spawner.h"

namespace hob {
    Entity::Entity(Engine& engine)
        : m_engine(engine) {}

    void Entity::enter_world() {
        if (m_is_in_world) {
            return;
        }

        m_is_in_world = true;
        for (auto& component : m_components) {
            component->enter_world();
        }
    }

    void Entity::exit_world() {
        if (!m_is_in_world) {
            return;
        }

        m_is_in_world = false;
        for (auto& component : m_components) {
            component->exit_world();
        }
    }

    void Entity::enter_play() {
        if (m_is_in_play) {
            return;
        }

        m_is_in_play = true;
        for (auto& component : m_components) {
            component->enter_play();
        }

        if (m_is_ticking_request) {
            get_engine().get_entity_spawner().register_ticking_entity(this);
        }
    }

    void Entity::exit_play() {
        if (!m_is_in_play) {
            return;
        }

        if (is_ticking()) {
            get_engine().get_entity_spawner().unregister_ticking_entity(this);
        }

        m_is_in_play = false;
        for (auto& component : m_components) {
            component->exit_play();
        }
    }

    void Entity::tick(float delta_time) {
        for (auto& component : m_components) {
            component->tick(delta_time);
        }
    }

    void Entity::physics_tick(float fixed_delta_time) {
        for (auto& component : m_components) {
            component->physics_tick(fixed_delta_time);
        }
    }

    void Entity::late_tick(float delta_time) {
        for (auto& component : m_components) {
            component->late_tick(delta_time);
        }
    }

    void Entity::debug_draw_tick(float delta_time) {
        for (auto& component : m_components) {
            component->debug_draw_tick(delta_time);
        }
    }

    void Entity::on_collision_enter(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_collision_enter(other_collider);
        }
    }

    void Entity::on_collision_exit(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_collision_exit(other_collider);
        }
    }

    void Entity::on_trigger_enter(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_trigger_enter(other_collider);
        }
    }

    void Entity::on_trigger_exit(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_trigger_exit(other_collider);
        }
    }

    std::string Entity::to_string() const {
        std::string result = std::format("Entity(name = {}, id = {}, in_play = {}, ticking = {})",
                                         get_display_name(),
                                         get_id(),
                                         is_in_play(),
                                         is_ticking());

        if (const TransformComponent* transform = get_transform()) {
            result += std::format("\n  position = {}, rotation = {}, scale = {}",
                                  transform->get_position().to_string(),
                                  transform->get_rotation(),
                                  transform->get_scale().to_string());
        }

        result += std::format("\n  components ({}):", m_components.size());
        for (const auto& component : m_components) {
            result += std::format("\n    - {}", component->to_string());
        }

        return result;
    }

    Engine& Entity::get_engine() const {
        return m_engine;
    }

    EntityId Entity::get_id() const {
        return m_id;
    }

    void Entity::set_id(EntityId id) {
        m_id = id;
        m_fallback_display_name.clear();
    }

    const std::string& Entity::get_name() const {
        return m_name;
    }

    void Entity::set_name(std::string name) {
        m_name = std::move(name);
    }

    const std::string& Entity::get_prefab_name() const {
        return m_prefab_name;
    }

    void Entity::set_prefab_name(std::string name) {
        m_prefab_name = std::move(name);
    }

    const std::string& Entity::get_display_name() const {
        if (!m_name.empty()) {
            return m_name;
        }

        if (!m_prefab_name.empty()) {
            return m_prefab_name;
        }

        if (m_fallback_display_name.empty()) {
            m_fallback_display_name = std::format("Entity {}", m_id);
        }

        return m_fallback_display_name;
    }

    bool Entity::is_in_world() const {
        return m_is_in_world;
    }

    bool Entity::is_in_play() const {
        return m_is_in_play;
    }

    bool Entity::is_ticking() const {
        return m_tick_index != INVALID_TICK_INDEX;
    }

    void Entity::set_ticking(bool is_ticking) {
        m_is_ticking_request = is_ticking;

        if (is_in_play() && is_ticking != this->is_ticking()) {
            get_engine().get_entity_spawner().request_entity_ticking_sync(get_id());
        }
    }

    TransformComponent* Entity::get_transform() const {
        if (m_transform == nullptr) {
            m_transform = get_component<TransformComponent>();
        }

        return m_transform;
    }

    RigidbodyComponent* Entity::get_rigidbody() const {
        if (!m_rigidbody_resolved) {
            m_rigidbody = get_component<RigidbodyComponent>();
            m_rigidbody_resolved = true;
        }

        return m_rigidbody;
    }

    LuaScriptComponent* Entity::add_lua_component(std::string class_name) {
        if (LuaScriptComponent* existing = get_lua_component(class_name)) {
            log_duplicate_component(*existing);
            return existing;
        }

        return emplace_component<LuaScriptComponent>(std::move(class_name));
    }

    LuaScriptComponent* Entity::get_lua_component(std::string_view class_name) const {
        return get_component<LuaScriptComponent>([class_name](const LuaScriptComponent* comp) {
            return comp->get_class_name() == class_name;
        });
    }

    void Entity::sort_components() {
        std::stable_sort(m_components.begin(), m_components.end(), [](const auto& a, const auto& b) {
            return a->get_priority() < b->get_priority();
        });
    }

    void Entity::log_duplicate_component(const Component& comp) const {
        log::engine.error("Entity(name = {}, id = {}) already has {}; returning the existing one",
                          get_display_name(),
                          get_id(),
                          comp.to_string());
    }
} // namespace hob
