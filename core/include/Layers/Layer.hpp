#pragma once

#include "SGF_Core.hpp"
#include "Events/InputEvents.hpp"

namespace SGF {
    class RenderEvent {
    public:
        inline RenderEvent(const Window& win, double dTime, const CommandList& commandList, VkExtent2D framebufferSize) 
            : window(&win), deltaTime(dTime), commands(&commandList), renderArea(framebufferSize) {}
        inline double getTime() const { return deltaTime; }
        inline const CommandList& getCommands() const { return *commands; }
        inline VkExtent2D getFramebufferSize() const { return renderArea; }
        inline const Window& getWindow() const { return *window; }
        inline void addWait(VkSemaphore semaphore, VkPipelineStageFlags waitStage) {
            waitSemaphores.push_back(semaphore);
            waitStages.push_back(waitStage);
        }
        inline void addSignal(VkSemaphore signalSemaphore) {
            signalSemaphores.push_back(signalSemaphore);
        }
        inline const std::vector<VkSemaphore>& getSignal() const {
            return signalSemaphores;
        }
        inline const std::vector<VkSemaphore>& getWait() const {
            return waitSemaphores;
        }
        inline const std::vector<VkPipelineStageFlags>& getWaitStages() const {
            return waitStages;
        }
    private:
        const Window* window;
        const CommandList* commands;
        double deltaTime;
        const VkExtent2D renderArea;
        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkSemaphore> signalSemaphores;
        std::vector<VkPipelineStageFlags> waitStages;
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
        inline virtual void onRender(RenderEvent& event) {}
        inline virtual void onUpdate(const UpdateEvent& event) {}
        inline virtual void onAttach() {}
        inline virtual void onDetach() {}
        inline const char* getName() const { return debugName.c_str(); }
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
        inline void onRender(RenderEvent& event) override {
            SGF::info("hello world: ", number, " ", getName());
        }
        inline void onUpdate(const UpdateEvent& event) override {
            SGF::info("how are you???, ", number, getName());
        }
    private:
        uint32_t number;
    };
}