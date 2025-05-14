#pragma once

#include "SGF_Core.hpp"
#include "Events/Inputevents.hpp"

namespace SGF {
    class RenderEvent {
    public:
        inline RenderEvent(double dTime) : deltaTime(dTime) {}
    private:
        double deltaTime;
    };
    class UpdateEvent {
    public:
        inline UpdateEvent(double dTime) : deltaTime(dTime) {}
    private:
        double deltaTime;
    };
    class Layer {
    public:
        inline Layer(const char* name) : debugName(name) {}
        inline virtual bool onKeyPress(const KeyPressedEvent& event) { return false; }
        inline virtual bool onKeyRelease(const KeyReleasedEvent& event) { return false; }
        inline virtual bool onKeyRepeat(const KeyRepeatEvent& event) { return false; }
        inline virtual bool onMouseMove(const MouseMovedEvent& event) { return false; }
        inline virtual bool onMousePress(const MousePressedEvent& event) { return false; }
        inline virtual bool onMouseRelease(const MouseReleasedEvent& event) { return false; }
        inline virtual bool onMouseScroll(const MouseScrollEvent& event) { return false; }
        inline virtual bool onKeyTyped(const KeyTypedEvent& event) { return false; }
        inline virtual void onRender(const RenderEvent& event) {}
        inline virtual void onUpdate(const UpdateEvent& event) {}
        inline virtual void onAttach() {}
        inline virtual void onDetach() {}
        inline const char* getName() { return debugName.c_str(); }
        virtual ~Layer();
    protected:
        Layer(const Layer& other);
        Layer(Layer&& other) noexcept;
        std::string debugName;
    private:
        friend LayerStack;
        size_t layerIndex = UINT64_MAX;
    };
    class TestLayer : public Layer {
    public:
        inline TestLayer(uint32_t num) : number(num), Layer("test_layer") {}
        inline void onRender(const RenderEvent& event) override {
            SGF::info("hello world: ", number, " ", getName());
        }
        inline void onUpdate(const UpdateEvent& event) override {
            SGF::info("how are you???, ", number, getName());
        }
    private:
        uint32_t number;
    };
}