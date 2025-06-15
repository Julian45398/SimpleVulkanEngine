#pragma once

#include "SGF_Core.hpp"
#include "Events/Event.hpp"
#include "Events/InputEvents.hpp"
#include "Layers/Layer.hpp"

namespace SGF {
    class LayerStack {
    public:
        void push(Layer* layer);
        void pop();
        void pushOverlay(Layer* layer);
        void popOverlay();
        void insert(Layer* layer, size_t index);
        void erase(size_t index);
        void clear();

        // Events:
        void onEvent(RenderEvent& event);
        void onEvent(const UpdateEvent& event);
        void onEvent(const KeyPressedEvent& event);
        void onEvent(const KeyReleasedEvent& event);
        void onEvent(const KeyRepeatEvent& event);
        void onEvent(const KeyTypedEvent& event);
        void onEvent(const MousePressedEvent& event);
        void onEvent(const MouseReleasedEvent& event);
        void onEvent(const MouseMovedEvent& event);
        void onEvent(const MouseScrollEvent& event);
    private:
        static LayerStack s_MainStack;
    public:
        inline static void Push(Layer* pLayer) { s_MainStack.push(pLayer); }
        inline static void Pop() { s_MainStack.pop(); }
        inline static void PushOverlay(Layer* pLayer) { s_MainStack.pushOverlay(pLayer); }
        inline static void PopOverlay() { s_MainStack.popOverlay(); }
        inline static void Insert(Layer* pLayer, size_t index) { s_MainStack.insert(pLayer, index); }
        //inline static void Erase(size_t index) { s_MainStack.erase(index); }
        inline static void Clear() { s_MainStack.clear(); }
        template<typename T>
        inline static void OnEvent(const T& event) { s_MainStack.onEvent(event); }
        template<typename T>
        inline static void OnEvent(T& event) { s_MainStack.onEvent(event); }
    private:
        friend Layer;
        std::vector<Layer*> layers;
        size_t layerCount = 0;
    };
}