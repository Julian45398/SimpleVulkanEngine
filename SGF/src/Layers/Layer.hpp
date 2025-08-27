#pragma once

#include "SGF_Core.hpp"
#include "LayerEvents.hpp"

namespace SGF {
    class RenderEvent {
    public:
        inline RenderEvent(double dTime, const CommandList& commandList, VkExtent2D framebufferSize) 
            : deltaTime(dTime), commands(&commandList), renderArea(framebufferSize) {}
        inline double GetTime() const { return deltaTime; }
        inline const CommandList& GetCommands() const { return *commands; }
        inline VkExtent2D GetFramebufferSize() const { return renderArea; }
        inline void AddWait(VkSemaphore semaphore, VkPipelineStageFlags waitStage) {
            waitSemaphores.push_back(semaphore);
            waitStages.push_back(waitStage);
        }
        inline void AddSignal(VkSemaphore signalSemaphore) {
            signalSemaphores.push_back(signalSemaphore);
        }
        inline const std::vector<VkSemaphore>& GetSignal() const {
            return signalSemaphores;
        }
        inline const std::vector<VkSemaphore>& GetWait() const {
            return waitSemaphores;
        }
        inline const std::vector<VkPipelineStageFlags>& GetWaitStages() const {
            return waitStages;
        }
    private:
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
        inline double GetDeltaTime() const { return deltaTime; }
    private:
        double deltaTime;
    };
    class Layer {
    public:
        inline Layer(const char* name) : debugName(name) {}

        // Input events:
        inline virtual bool OnEvent(const KeyPressedEvent& event) { return false; }
        inline virtual bool OnEvent(const KeyReleasedEvent& event) { return false; }
        inline virtual bool OnEvent(const KeyRepeatEvent& event) { return false; }
        inline virtual bool OnEvent(const MouseMovedEvent& event) { return false; }
        inline virtual bool OnEvent(const MousePressedEvent& event) { return false; }
        inline virtual bool OnEvent(const MouseReleasedEvent& event) { return false; }
        inline virtual bool OnEvent(const MouseScrollEvent& event) { return false; }
        inline virtual bool OnEvent(const KeyTypedEvent& event) { return false; }

        // Window events:
        inline virtual void OnEvent(const WindowIconifyEvent& event) {}
        inline virtual void OnEvent(const WindowCloseEvent& event) {}
        inline virtual void OnEvent(const WindowOpenEvent& event) {}
        inline virtual void OnEvent(const WindowResizeEvent& event) {}

        // Device events:
        inline virtual void OnEvent(const DeviceDestroyEvent& event) {}
        inline virtual void OnEvent(const DeviceCreateEvent& event) {}

        // Looping events:
        inline virtual void OnEvent(RenderEvent& event) {}
        inline virtual void OnEvent(const UpdateEvent& event) {}

        inline virtual void OnAttach() {}
        inline virtual void OnDetach() {}
        inline const char* GetName() const { return debugName.c_str(); }
        virtual ~Layer();
    protected:
        Layer(const Layer& other);
        Layer(Layer&& other) = delete;
        std::string debugName;
    private:
        friend LayerStack;
        size_t layerIndex = UINT64_MAX;
    };
    class TestLayer : public Layer {
    public:
        inline TestLayer(uint32_t num) : number(num), Layer("test_layer") {}
        inline void OnEvent(RenderEvent& event) override {
            SGF::info("hello world: ", number, " ", GetName());
        }
        inline void OnEvent(const UpdateEvent& event) override {
            SGF::info("how are you???, ", number, GetName());
        }
    private:
        uint32_t number;
    };
}