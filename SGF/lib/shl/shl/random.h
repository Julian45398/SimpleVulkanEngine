#pragma once

#include <stdint.h>

namespace shl {
	inline uint32_t randomInt32(uint32_t seed) {
		uint32_t state = seed * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}
	inline uint64_t randomInt64(uint64_t seed) {
		union SeedM
		{
			uint64_t asInt64;
			uint32_t asInt32[2];
		};
		SeedM converter;
		converter.asInt64 = seed;
		SeedM random_number;
		random_number.asInt32[0] = randomInt32(converter.asInt32[0]);
		random_number.asInt32[1] = randomInt32(converter.asInt32[1]);
		return random_number.asInt64;
	}
}