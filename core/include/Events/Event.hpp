#pragma once

#include "SGF_Core.hpp"

namespace SGF {
    namespace EventManager {
        template<typename EVENT_TYPE>
        class EventHandler {
        public:
            template<typename USER>
            inline static void AddListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
                FunctionUserPair obs{ (EventFunction)func, (void*)user };
                GetListeners().push_back(obs);
            }
            template<typename USER>
            inline static bool RemoveListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
                FunctionUserPair obs{ (EventFunction)func, (void*)user };
                auto& listeners = GetListeners();
                for (size_t i = 0; i < listeners.size(); ++i) {
                    if (obs == listeners[i]) {
                        listeners.erase(listeners.begin() + i);
                        return true;
                    }
                }
                return false;
            }
            inline static void ClearListeners() {
                auto& listeners = GetListeners();
                listeners.clear();
                listeners.shrink_to_fit();
            }
            inline static void Dispatch(const EVENT_TYPE& event) {
                auto& listeners = GetListeners();
                for (size_t i = listeners.size(); i > 0; --i) {
                    SGF::debug("dispatching event: index ", i);
                    listeners[i-1].func(event, listeners[i-1].user);
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
            inline static std::vector<FunctionUserPair>& GetListeners() {
                static std::vector<FunctionUserPair> listeners;
                return listeners;
            }
            inline static size_t& GetRemovalCount() {
                static size_t removalCount;
                return removalCount;
            }
        };
        //==========================================
        template<typename EVENT_TYPE, typename USER>
        inline void AddListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
            EventHandler<EVENT_TYPE>::AddListener(func, user);
        }
        template<typename EVENT_TYPE, typename USER>
        inline bool RemoveListener(void(*func)(const EVENT_TYPE&, USER*), USER* user) {
            return EventHandler<EVENT_TYPE>::RemoveListener(func, user);
        }
        template<typename EVENT_TYPE>
        inline void Dispatch(const EVENT_TYPE& event) {
            EventHandler<EVENT_TYPE>::Dispatch(event);
        }
        template<typename EVENT_TYPE>
        inline void Clear() {
            EventHandler<EVENT_TYPE>::ClearListeners();
        }
        
    }
}
