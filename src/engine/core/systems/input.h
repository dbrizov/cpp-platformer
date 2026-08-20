#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL_events.h>

#include "engine/math/vector2.h"
#include "input_config.h"

struct SDL_Gamepad;

namespace hob {
    class Renderer;

    enum class InputEventType {
        Axis,
        Pressed,
        Released,
    };

    struct InputEvent {
        std::string_view name;
        InputEventType type;
        float axis_value;

        InputEvent(std::string_view ev_name, InputEventType ev_type, float ev_axis_value);
    };

    using InputEventHandler = std::function<void(const InputEvent&)>;
    using InputEventHandlerId = int32_t;
    using InputEventHandlerIndex = uint32_t;

    constexpr InputEventHandlerId INVALID_INPUT_EVENT_HANDLER_ID = -1;

    class Input {
        const Renderer& m_renderer;

        struct HandlerEntry {
            InputEventHandlerId handler_id;
            InputEventHandler handler;
        };

        InputEventHandlerId m_next_handler_id = 0;
        std::vector<HandlerEntry> m_handlers;
        std::unordered_map<InputEventHandlerId, InputEventHandlerIndex> m_handler_index_by_id;

        InputConfig m_input_config;

        struct ActionEntry {
            std::string_view name;
            std::vector<uint32_t> source_indices;
        };

        struct AxisEntry {
            std::string_view name;
            const AxisConfig* config = nullptr;
            std::vector<uint32_t> positive_indices;
            std::vector<uint32_t> negative_indices;
            float ramped_value = 0.0f; // digital accel/decel ramp, carried across frames
        };

        std::vector<ActionEntry> m_actions;
        std::vector<AxisEntry> m_axes;

        // Digital source edge tracking, parallel to m_digital_sources.
        std::vector<InputSource> m_digital_sources;
        std::vector<uint8_t> m_down_this_frame;
        std::vector<uint8_t> m_down_last_frame;

        // Per-frame device state.
        bool m_is_mouse_over_game_window = false;
        uint32_t m_mouse_button_mask = 0;
        Vector2 m_mouse_screen_position;
        Vector2 m_mouse_motion_delta;
        float m_mouse_wheel_delta = 0.0f;

        // Single gamepad (player 1) with hotplug. Null when none connected.
        SDL_Gamepad* m_gamepad = nullptr;
        uint32_t m_gamepad_id = 0;

    public:
        explicit Input(const Renderer& renderer);
        ~Input();

        Input(const Input&) = delete;
        Input& operator=(const Input&) = delete;

        Input(Input&&) = delete;
        Input& operator=(Input&&) = delete;

        void process_event(const SDL_Event& event);
        void tick(float delta_time);
        void end_frame(bool is_game_input_active);

        InputEventHandlerId add_input_event_handler(InputEventHandler handler);
        bool remove_input_event_handler(InputEventHandlerId id);

        Vector2 get_mouse_screen_position() const;
        bool is_mouse_over_game_window() const;
        bool is_gamepad_connected() const;

    private:
        void reset_mouse_state();
        void update_mouse_state();
        void update_down_states();

        void build_dispatch_tables();
        void dispatch_event(const InputEvent& event) const;
        void dispatch_actions();
        void dispatch_axes(float delta_time);

        bool is_source_down(const InputSource& source) const;
        float read_analog_source(const InputSource& source) const;

        void open_gamepad(uint32_t gamepad_id);
        void close_gamepad();
        void adopt_any_gamepad();

        static uint32_t pack_source(const InputSource& source);
    };
} // namespace hob
