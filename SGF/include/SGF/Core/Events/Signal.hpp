#pragma once

#include <stddef.h>
#include <vector>

namespace SGF {
	template<typename EVENT>
    class Signal {
		struct Listener {
			uint64_t id;
			void* pData;
			bool (*func)(void*, const EVENT&);
			bool operator()(const EVENT& e) { return func(pData, e); }
		};
		class Connection {
			friend class Signal<EVENT>;
		public:
			inline void Disconnect() { Signal<EVENT>::Unsubscribe(*this); }
			inline ~Connection() { Disconnect(); };
		private:
			inline Connection(uint64_t id) : id(id) {}
			uint64_t id;
		};
    public:
		template<auto METHOD, typename T>
		inline static Connection Subscribe(T* instance) {
			s_NextListenerId++;
			s_Listeners.push_back({
				s_NextListenerId,
				instance,
				[](void* obj, const EVENT& e) -> bool
				{
					return (static_cast<T*>(obj)->*METHOD)(e);
				}
				});
		}
		template<auto FUNC>
		inline static Connection Subscribe() {
			s_NextListenerId++;
			s_Listeners.push_back({
				s_NextListenerId,
				nullptr,
				[](void* obj, const EVENT& e) -> bool
				{
					return FUNC(e);
				}
				});
		}
		inline static bool Emit(const EVENT& event) {
			bool handled = false;
			bool removeListener = false;
			for (size_t i = s_Listeners.size(); i > 0; --i)  {
				if (s_Listeners[i-1].id == 0) {
					removeListener = true;
					continue;
				}
				handled = s_Listeners[i - 1](event);
				if (s_Listeners[i - 1](event)) {
					handled = true;
					break;
				}
			}
			if (removeListener) {
				for (size_t i = s_Listeners.size(); i > 0; --i) {
					if (s_Listeners[i - 1].id == 0) {
						s_Listeners.erase(s_Listeners.begin() + (i - 1));
						continue;
					}
				}
			}
			return handled;
		}
		inline static bool EmitBackwards(const EVENT& event) {
			bool handled = false;
			for (size_t i = s_Listeners.size(); i > 0; --i)  {
				if (s_Listeners[i-1].id == 0) {
					s_Listeners.erase(s_Listeners.begin() + (i - 1));
					continue;
				}
				if (s_Listeners[i - 1](event)) {
					handled = true;
					break;
				}
			}
			return handled;
		}
		inline static bool Unsubscribe(Connection id) {
			for (size_t i = 0; i < s_Listeners.size(); ++i) {
				if (s_Listeners[i].id == id.id) {
					s_Listeners[i].id = 0;
					return true;
				}
			}
			return false;
		}
    private:
		inline static uint64_t s_NextListenerId = 0;
		static std::vector<Listener> s_Listeners;
    };
}