#pragma once

#include "SGF_Core.hpp"

namespace SGF {
    class Event {};

    class HandleableEvent : Event {
    public:
        HandleableEvent() = default;
        inline void setHandled() { handled = true; }
        inline bool isHandled() { return handled; }
    private:
        bool handled = false;
    };

    template<typename T>
    class EventMessenger {
    public:
        static_assert(std::is_base_of<Event, T>::value, "Event must be derived from the base event-class!");
        EventMessenger() = default;
        template<typename USER>
        inline bool subscribeEvent(void(*func)(const T&, USER*), USER* user) {
            FunctionUserPair obs{ (EventFunction)func, (void*)user };
            listeners.insert(obs);
            return true;
        }
        template<typename USER>
        inline bool unsubscribe(void(*func)(const T&, USER*), USER* user) {
            FunctionUserPair obs{ (EventFunction)func, (void*)user };
            listeners.erase(obs);
        }
        inline void dispatch(const T& event) {
            for (auto obs : listeners) {
                obs.func(event, obs.user);
            }
        }
        inline void clear() {
            listeners.clear();
        }
    private:
        typedef void (*EventFunction)(const T& evt, void* data);
        struct FunctionUserPair {
            EventFunction func;
            void* user;
            bool operator<(const FunctionUserPair& other) {
                if (func == other.func) {
                    return user < other.user;
                }
                else {
                    return func < other.func;
                }
            }
            bool operator==(const FunctionUserPair& other) {
                return func == other.func && user == other.user;
            }
        };
        std::set<FunctionUserPair> listeners;
    };

    template<typename T>
    class EventMessengerStack {
        static_assert(std::is_base_of<Event, T>::value, "Event must be derived from the base event-class!");
    public:
        EventMessengerStack() = default;
        template<typename USER>
        inline void pushListener(void(*func)(T&, USER*), USER* user) {
            FunctionUserPair obs{ (EventFunction)func, (void*)user };
            listeners.push(obs);
        }
        inline void popListener() {
            listeners.pop();
        }
        inline void dispatch(T& event) {
            for (auto obs : listeners) {
                obs.func(event, obs.user);
            }
        }
        inline void reversDispatch(T& event) {
            //static_assert(std::is_baseof<Event, T>);
            for (size_t i = listeners.size() - 1; i >= 0; --i) {
                listeners[i].func(event, listeners[i].user);
            }
        }
        inline void clear() {
            listeners.clear();
            listeners.shrink_to_fit();
        }
    private:
        typedef void (*EventFunction)(T& evt, void* data);
        struct FunctionUserPair {
            EventFunction func;
            void* user;
            bool operator==(const FunctionUserPair& other) {
                return func == other.func && user == other.user;
            }
        };
        std::stack<FunctionUserPair> listeners;
    };
}
