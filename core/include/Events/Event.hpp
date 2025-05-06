#pragma once

#include "SGF_Core.hpp"

namespace SGF {
    namespace EventManager {
        template<typename EVENT_TYPE>
        class EventHandler {
        public:
            template<typename USER>
            inline static void addListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
                FunctionUserPair obs{ (EventFunction)func, (void*)user };
                getListeners().push_back(obs);
            }
            template<typename USER>
            inline static bool removeListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
                FunctionUserPair obs{ (EventFunction)func, (void*)user };
                auto& listeners = getListeners();
                for (size_t i = 0; i < listeners.size(); ++i) {
                    if (obs == listeners[i]) {
                        listeners.erase(listeners.begin() + i);
                        return true;
                    }
                }
                return false;
            }
            inline static void clearListeners() {
                auto& listeners = getListeners();
                listeners.clear();
                listeners.shrink_to_fit();
            }
            inline static void dispatch(const EVENT_TYPE& event) {
                for (auto& listener : getListeners()) {
                    listener.func(event, listener.user);
                }
            }
        private:
            using EventFunction = void(*)(const EVENT_TYPE&, void*);
            struct FunctionUserPair {
                EventFunction func;
                void* user;
                inline bool operator<(const FunctionUserPair& other) {
                    if (func == other.func) {
                        return user < other.user;
                    }
                    else {
                        return func < other.func;
                    }
                }
                inline bool operator==(const FunctionUserPair& other) {
                    return func == other.func && user == other.user;
                }
            };
            inline static std::vector<FunctionUserPair>& getListeners() {
                static std::vector<FunctionUserPair> listeners;
                return listeners;
            }
        };
        //==========================================
        template<typename EVENT_TYPE, typename USER>
        inline void addListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
            EventHandler<EVENT_TYPE>::addListener(func, user);
        }
        template<typename EVENT_TYPE>
        inline void dispatch(const EVENT_TYPE& event) {
            EventHandler<EVENT_TYPE>::dispatch(event);
        }
        template<typename EVENT_TYPE>
        inline void clear() {
            EventHandler<EVENT_TYPE>::clearListeners();
        }
        template<typename EVENT_TYPE, typename USER>
        inline bool removeListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
            return EventHandler<EVENT_TYPE>::removeListener(func, user);
        }
    }
}
