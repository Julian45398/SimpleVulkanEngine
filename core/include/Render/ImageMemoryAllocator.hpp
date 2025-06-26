#pragma once

#include "SGF_Core.hpp"

namespace SGF {
	struct TextureImage {
		VkImage image;
		VkImageView view;
	};

	class ImageMemoryAllocator {
	public:
		static const VkDeviceSize REGION_SIZE = 16384 * 8192;
	private:
		struct MemRegion {
			uint32_t size;
			uint32_t offset;
			uint32_t regionIndex;
		};
		struct ImageRegion {
			TextureImage image;
			MemRegion region;
		};
		std::vector<VkDeviceMemory> allocatedRegions;
		std::vector<MemRegion> freeRegions;
		std::vector<ImageRegion> imageRegions;
	public:
		const TextureImage createImage(uint32_t width, uint32_t height);

		void destroyImage(const TextureImage& texture);

		inline size_t getAllocatedSize() {
			return allocatedRegions.size() * REGION_SIZE;
		}

		void defragmentMemory();

		~ImageMemoryAllocator();
	};
}