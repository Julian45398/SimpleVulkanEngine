#pragma once

namespace SGF {
	// Memory Size Constants (Powers of 2)
	enum class MemorySize : size_t {
		ZERO = 0,
		// Bytes
		BYTE_1 = 1ULL,
		BYTE_2 = 1ULL << 1,   // 2 B
		BYTE_4 = 1ULL << 2,   // 4 B
		BYTE_8 = 1ULL << 3,   // 8 B
		BYTE_16 = 1ULL << 4,   // 16 B
		BYTE_32 = 1ULL << 5,   // 32 B
		BYTE_64 = 1ULL << 6,   // 64 B
		BYTE_128 = 1ULL << 7,   // 128 B
		BYTE_256 = 1ULL << 8,   // 256 B
		BYTE_512 = 1ULL << 9,   // 512 B

		// Kilobytes (KB)
		KB_1 = 1ULL << 10,  // 1 KB
		KB_2 = 1ULL << 11,  // 2 KB
		KB_4 = 1ULL << 12,  // 4 KB
		KB_8 = 1ULL << 13,  // 8 KB
		KB_16 = 1ULL << 14,  // 16 KB
		KB_32 = 1ULL << 15,  // 32 KB
		KB_64 = 1ULL << 16,  // 64 KB
		KB_128 = 1ULL << 17,  // 128 KB
		KB_256 = 1ULL << 18,  // 256 KB
		KB_512 = 1ULL << 19,  // 512 KB
		// Megabytes (MB)
		MB_1 = 1ULL << 20,  // 1 MB
		MB_2 = 1ULL << 21,  // 2 MB
		MB_4 = 1ULL << 22,  // 4 MB
		MB_8 = 1ULL << 23,  // 8 MB
		MB_16 = 1ULL << 24,  // 16 MB
		MB_32 = 1ULL << 25,  // 32 MB
		MB_64 = 1ULL << 26,  // 64 MB
		MB_128 = 1ULL << 27,  // 128 MB
		MB_256 = 1ULL << 28,  // 256 MB
		MB_512 = 1ULL << 29,  // 512 MB
		// Gigabytes (GB)
		GB_1 = 1ULL << 30,  // 1 GB
		GB_2 = 1ULL << 31,  // 2 GB
		GB_4 = 1ULL << 32,  // 4 GB
		GB_8 = 1ULL << 33,  // 8 GB
		GB_16 = 1ULL << 34,  // 16 GB
		GB_32 = 1ULL << 35,  // 32 GB
		GB_64 = 1ULL << 36,  // 64 GB
	};
}
