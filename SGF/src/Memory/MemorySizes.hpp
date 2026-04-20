#pragma once

namespace SGF {
	// Memory Size Constants (Powers of 2)
	namespace MemorySize {
		// Bytes
		constexpr size_t BYTE_1 = 1ULL;
		constexpr size_t BYTE_2 = 1ULL << 1;   // 2 B
		constexpr size_t BYTE_4 = 1ULL << 2;   // 4 B
		constexpr size_t BYTE_8 = 1ULL << 3;   // 8 B
		constexpr size_t BYTE_16 = 1ULL << 4;   // 16 B
		constexpr size_t BYTE_32 = 1ULL << 5;   // 32 B
		constexpr size_t BYTE_64 = 1ULL << 6;   // 64 B
		constexpr size_t BYTE_128 = 1ULL << 7;   // 128 B
		constexpr size_t BYTE_256 = 1ULL << 8;   // 256 B
		constexpr size_t BYTE_512 = 1ULL << 9;   // 512 B

		// Kilobytes (KB)
		constexpr size_t KB_1 = 1ULL << 10;  // 1 KB
		constexpr size_t KB_2 = 1ULL << 11;  // 2 KB
		constexpr size_t KB_4 = 1ULL << 12;  // 4 KB
		constexpr size_t KB_8 = 1ULL << 13;  // 8 KB
		constexpr size_t KB_16 = 1ULL << 14;  // 16 KB
		constexpr size_t KB_32 = 1ULL << 15;  // 32 KB
		constexpr size_t KB_64 = 1ULL << 16;  // 64 KB
		constexpr size_t KB_128 = 1ULL << 17;  // 128 KB
		constexpr size_t KB_256 = 1ULL << 18;  // 256 KB
		constexpr size_t KB_512 = 1ULL << 19;  // 512 KB

		// Megabytes (MB)
		constexpr size_t MB_1 = 1ULL << 20;  // 1 MB
		constexpr size_t MB_2 = 1ULL << 21;  // 2 MB
		constexpr size_t MB_4 = 1ULL << 22;  // 4 MB
		constexpr size_t MB_8 = 1ULL << 23;  // 8 MB
		constexpr size_t MB_16 = 1ULL << 24;  // 16 MB
		constexpr size_t MB_32 = 1ULL << 25;  // 32 MB
		constexpr size_t MB_64 = 1ULL << 26;  // 64 MB
		constexpr size_t MB_128 = 1ULL << 27;  // 128 MB
		constexpr size_t MB_256 = 1ULL << 28;  // 256 MB
		constexpr size_t MB_512 = 1ULL << 29;  // 512 MB

		// Gigabytes (GB)
		constexpr size_t GB_1 = 1ULL << 30;  // 1 GB
		constexpr size_t GB_2 = 1ULL << 31;  // 2 GB
		constexpr size_t GB_4 = 1ULL << 32;  // 4 GB
		constexpr size_t GB_8 = 1ULL << 33;  // 8 GB
		constexpr size_t GB_16 = 1ULL << 34;  // 8 GB
	}
}
