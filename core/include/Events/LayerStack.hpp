#pragma once

#include "SGF_Core.hpp"
#include <vector>
#include <stack>

namespace SGF {
    class Layer {
    public:
        void onAttach();
    private:
#ifndef NDEBUG
        size_t layerIndex;
#endif
    };
    class Event {
    };
    template<typename T>
    class StackedEventHandler {
        typedef bool (*EventFunction)(T& event, void* user);
        std::stack<std::pair<EventFunction, void*>> functionStack;

        void pushListener(EventFunction func, void* listener);
        void pop();
    };
    class RenderEvent : Event {
    };
    class MouseButtonPressedEvent : Event {

    };
    class ExampleLayer : Layer {
        inline static bool onRender(RenderEvent& event, Layer* layer) {
            ExampleLayer& exlayer = *(ExampleLayer*)layer;
        }
    };
    class LayerStack {
        typedef bool (*RenderFunction)(RenderEvent& event, Layer* layer);
        typedef bool (*MouseButtonPressedFunction)(MouseButtonPressedEvent& event, Layer* layer);
        std::stack<MouseButtonPressedFunction> onMouseEvent;
        std::stack<RenderFunction> onRender;
        template<typename T>
        void attach(const T& layer) {
            T.onAttach();
        }
    };
}