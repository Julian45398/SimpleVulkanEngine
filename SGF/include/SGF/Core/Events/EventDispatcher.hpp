#pragma once

#include "Event.hpp"
#include <functional>

namespace SGF {
	class EventDispatcher {
		template<typename EVENT_TYPE>
		using EventFn = std::function<bool(const EVENT_TYPE&)>;
	public:
		inline EventDispatcher(Event& event) : m_Event(&event) {}

		template<typename EVENT_TYPE>
		inline bool Dispatch(const EventFn<EVENT_TYPE>& func) {
			if (m_Event->GetType() == EVENT_TYPE::GetStaticType()) {
				return func(*(EVENT_TYPE*)m_Event);
			}
			return false;
		}
	private:
		Event* m_Event;
	};
}