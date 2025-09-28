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
		void AllocateNewPage();
		const TextureImage CreateImage(uint32_t width, uint32_t height);
		const TextureImage CreateDummyImage(uint32_t width, uint32_t height);

		void DestroyImage(const TextureImage& texture);

		inline size_t GetAllocatedSize() const { return allocatedRegions.size() * REGION_SIZE; }
		inline size_t GetUsedMemorySize() const { size_t us = 0; for (auto& r : imageRegions) { us += r.region.size; } return us; }

		void DefragmentMemory();

		~ImageMemoryAllocator();
	};
}