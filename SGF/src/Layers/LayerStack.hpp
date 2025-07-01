#pragma once

#include "SGF_Core.hpp"
#include "Events/Event.hpp"
#include "Layers/Layer.hpp"

namespace SGF {
    class LayerStack {
    public:
        void Push(Layer* layer);
        void Pop();
        void PushOverlay(Layer* layer);
        void PopOverlay();
        void Insert(Layer* layer, size_t index);
        void Erase(size_t index);
        void Clear();

        // Events:
        void OnEvent(RenderEvent& event);
        void OnEvent(const UpdateEvent& event);
        void OnEvent(const KeyPressedEvent& event);
        void OnEvent(const KeyReleasedEvent& event);
        void OnEvent(const KeyRepeatEvent& event);
        void OnEvent(const KeyTypedEvent& event);
        void OnEvent(const MousePressedEvent& event);
        void OnEvent(const MouseReleasedEvent& event);
        void OnEvent(const MouseMovedEvent& event);
        void OnEvent(const MouseScrollEvent& event);
        template<typename EVENT>
        inline void OnEvent(const EVENT& event) const {
            for (size_t i = layers.size(); i != 0; --i) {
                layers[i - 1]->OnEvent(event);
            }
        }
    private:
        static LayerStack s_MainStack;
    public:
        inline static LayerStack& Get() { return s_MainStack; }
    private:
        friend Layer;
        std::vector<Layer*> layers;
        size_t layerCount = 0;
    };
}