#pragma once

#include "SGF_Core.hpp"
#include "Event.hpp"
#include "WindowEvents.hpp"
#include <type_traits>

namespace SGF {
    class LayerStack {
    public:
        typedef void(*OnAttachFn)(void*);
        typedef void(*OnDetachFn)(void*);
        template<typename T>
        inline void pushOverlay(T& layer) {
        }
        inline void popOverlay() {
        }
        template<typename T>
        inline void pushLayer(T& layer) {
        }
        template<typename T>
        inline void popLayer() {

        }
    private:
        EventMessengerStack<KeyPressedEvent> keyPressed;
        EventMessengerStack<KeyReleasedEvent> keyReleased;
        EventMessengerStack<KeyRepeatEvent> keyRepeat;
        EventMessengerStack<KeyTypedEvent> keyTyped;
        EventMessengerStack<MousePressedEvent> mousePressed;
        EventMessengerStack<MouseReleasedEvent> mouseRepeat;
        EventMessengerStack<MouseScrollEvent> mouseScroll;
        EventMessengerStack<MouseMovedEvent> mouseMove;
    };
}