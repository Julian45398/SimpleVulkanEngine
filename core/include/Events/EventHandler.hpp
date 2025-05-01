#pragma once

#include <vector>
#include <stdint.h>
#include <map>
#include <set>
#include "Logger.hpp"

namespace SGF {
    template<typename T>
    class EventMessenger {
    public:
        EventMessenger() = default;
        typedef void (*EventFunction)(T& event);

        inline void subscribeEvent(EventFunction func) {
            listeners.emplace(func);
            //listeners.emplace_back(func, listener);
        }
        inline void unsubscribe(EventFunction func) {
            listeners.erase(func);
        }
        inline bool dispatch(T& event) {
            bool handled = false;
            for (auto func : listeners) {
                func(event, listeners[i].user);
            }
            return handled;
        }
    private:
        std::set<EventFunction> listeners;
    };
    template<typename T>
    class EventHandler {
    public:
        EventHandler() = default;
        typedef bool (*EventFunction)(T&, void* user);
        class EventListener {
        public:
            inline EventListener(EventFunction function, void* listener) : func(function), user(listener) {}
            EventFunction func;
            void* user;
        };
        inline void subscribeEvent(EventFunction func, void* listener) {
            listeners.emplace_back(func, listener);
        }
        inline void unsubscribe(const void* user) {
            for (size_t i = listeners.size(); 0 <= i; --i) {
                if (listeners[i].user == user) {
                    listeners.erase(listeners.begin() + i);
                }
            }
        }
        inline bool dispatch(T& event) {
            bool handled = false;
            for (size_t i = 0; i < callbacks.size(); ++i) {
                handled = listeners[i].func(event, listeners[i].user) || handled;
            }
            return handled;
        }
    private:
        std::vector<EventListener> listeners;
    };

    struct MousePressedEvent {
        uint32_t xpos;
        uint32_t ypos;
        int button;
    };

    class TestListener {
    public:
        inline TestListener(uint32_t num, const char* listenerName) : number(num), name(listenerName) {}
        inline bool onMousePressed(MousePressedEvent& event) {
            SGF::debug("Mouse pressed!", event.button, " pos: {", event.xpos,", ", event.ypos, "} Listener name: ", name);
        }
    private:
        uint32_t number;
        const char* name;
    };

    bool onTestEvent(MousePressedEvent& event, void* listener) {
        TestListener& list = *(TestListener*)listener;
        list.onMousePressed(event);
        return true;
    }
    inline void testEvent() {
        TestListener test(5, "Name of Test listener!");
        EventHandler<MousePressedEvent> handler;
        handler.subscribeEvent([](MousePressedEvent& event, void* listener){
            TestListener& list = *(TestListener*)listener;
            list.onMousePressed(event);
            return true;
        }, &test);
        TestListener test2(115, "Test2!");
        handler.subscribeEvent(onTestEvent, &test2);

        MousePressedEvent event;
        event.button = 69;
        event.xpos = 420;
        event.ypos = 2000;
        if (handler.dispatch(event)) {
            SGF::debug("Event Handled!");
        }
        handler.unsubscribe(&test2);
        handler.unsubscribe(&test);
    }
}





