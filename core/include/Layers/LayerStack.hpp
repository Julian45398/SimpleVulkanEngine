#pragma once

#include "SGF_Core.hpp"
#include "Events/Event.hpp"
#include "Events/InputEvents.hpp"
#include "Layers/Layer.hpp"

namespace SGF {
    class LayerStack {
    public:
        static void push(Layer& layer);
        static void pop();
        static void pushOverlay(Layer& layer);
        static void popOverlay();
        static void insert(Layer& layer, size_t index);
        static void erase(Layer& layer);
        static void clear();

        // Events:
        static void onEvent(RenderEvent& event);
        static void onEvent(const UpdateEvent& event);
        static void onEvent(const KeyPressedEvent& event);
        static void onEvent(const KeyReleasedEvent& event);
        static void onEvent(const KeyRepeatEvent& event);
        static void onEvent(const KeyTypedEvent& event);
        static void onEvent(const MousePressedEvent& event);
        static void onEvent(const MouseReleasedEvent& event);
        static void onEvent(const MouseMovedEvent& event);
        static void onEvent(const MouseScrollEvent& event);
    private:
        friend Layer;
        inline static std::vector<Layer*> layers;
        static size_t layerCount;
    };
}