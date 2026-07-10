#pragma once

#include <vector>
#include <memory>

#include "Layer.hpp"

namespace SGF {
    class LayerStack {
    public:
        void Push(Layer* layer);
        template<class LAYER, typename... Args>
        inline void Push(Args&&... args) { layers.emplace_back(std::make_unique<LAYER>(std::forward<Args>(args)...)); }
		inline void Pop() { layers.pop_back(); }
        void PushOverlay(Layer* layer);
        void PopOverlay();
        template<class LAYER, typename... Args>
        inline void Insert(size_t index, Args&&... args) { layers.emplace((layers.begin() + index), std::make_unique<LAYER>(std::forward<Args>(args)...)); }
        void Clear();

        // Events:
        void OnEvent(const RenderEvent& event);
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
        friend Layer;
        std::vector<std::unique_ptr<Layer>> layers;
		std::unique_ptr<Layer> overlayLayer;
    };
}