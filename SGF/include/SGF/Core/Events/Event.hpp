#pragma once

#include <stdint.h>

namespace SGF {
    class Event {
    public:
		inline Event(uint32_t eventType) : m_Type(eventType) {}
		inline uint32_t GetType() const { return m_Type; }
	private:
		uint32_t m_Type;
    };
}
