#pragma once

#include "LayerEvents.hpp"

#include <SGF/Core/Events/Event.hpp>

namespace SGF {
    class Layer {
    public:
        inline Layer() : m_Active(false), m_RemoveRequested(false) {}

        inline virtual void OnAttach() {}
        inline virtual void OnDetach() {}

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
        inline virtual bool OnEvent(const WindowIconifyEvent& event) { return false; }
        inline virtual bool OnEvent(const WindowCloseEvent& event) { return false; }
        inline virtual bool OnEvent(const WindowOpenEvent& event) { return false; }
        inline virtual bool OnEvent(const WindowResizeEvent& event) { return false; }

        // Looping events:
        inline virtual bool OnEvent(const RenderEvent& event) {}
        inline virtual bool OnEvent(const UpdateEvent& event) {}

		inline virtual bool OnEvent(const Event& event) {}

		inline void RequestRemoval() { m_RemoveRequested = true; }
		inline bool RemovalRequested() const { return m_RemoveRequested; }

		inline void SetActive(bool active) { m_Active = active; }
		inline bool IsActive() const { return m_Active; }
    protected:
        Layer(const Layer& other) = delete;
        Layer(Layer&& other) = delete;
    private:
		bool m_Active;
		bool m_RemoveRequested;
    };
}