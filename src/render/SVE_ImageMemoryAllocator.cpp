#include "SVE_ImageMemoryAllocator.h"

#include "SVE_Backend.h"

const TextureImage SveImageMemoryAllocator::createImage(uint32_t width, uint32_t height) {
	TextureImage texture;
	MemRegion textureRegion;
	texture.image = SVE::createImage2D(width, height, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	auto memreq = SVE::getImageMemoryRequirements(texture.image);
	if (memreq.size > REGION_SIZE) {
		shl::logFatal("image memory requirement exceeds max byts size of: ", REGION_SIZE);
	}
	bool region_found = false;
	for (size_t i = 0; i < freeRegions.size(); ++i) {
		MemRegion region = freeRegions[i];
		if (memreq.size <= region.size) {
			region_found = true;
			region.size = (uint32_t)memreq.size;
			SVE::bindImageMemory(texture.image, allocatedRegions[region.regionIndex], region.offset);
			textureRegion = region;
			if (memreq.size != region.size) {
				freeRegions[i].offset += (uint32_t)memreq.size;
				freeRegions[i].size -= (uint32_t)memreq.size;
			}
			else {
				freeRegions.erase(freeRegions.begin() + i);
			}
			break;
		}
	}
	if (!region_found) {
		uint32_t size = (uint32_t)memreq.size;
		memreq.size = REGION_SIZE;
		VkDeviceMemory memory = SVE::allocateMemory(memreq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		allocatedRegions.push_back(memory);
		MemRegion region = { size, 0, (uint32_t)allocatedRegions.size() - 1 };
		SVE::bindImageMemory(texture.image, memory, region.offset);
		textureRegion = region;
		region.offset = size;
		region.size = (uint32_t)REGION_SIZE - size;
		freeRegions.push_back(region);
	}
	texture.view = SVE::createImageView2D(texture.image, VK_FORMAT_R8G8B8A8_SRGB);
	imageRegions.push_back({texture, textureRegion});

	return imageRegions.back().image;
}

void SveImageMemoryAllocator::destroyImage(const TextureImage& texture) {
	MemRegion region;
	for (uint32_t i = 0; i < imageRegions.size(); ++i) {
		if (imageRegions[i].image.image ==  texture.image) {
			region = imageRegions[i].region;
			imageRegions.erase(imageRegions.begin() + i);
			break;
		}
	}
	freeRegions.push_back(region);
	SVE::destroyImage(texture.image);
	SVE::destroyImageView(texture.view);
}

void SveImageMemoryAllocator::defragmentMemory() {
	shl::logWarn("Defragmentation of image allocator memory not implemented yet!");
}

SveImageMemoryAllocator::~SveImageMemoryAllocator() {
	for (const auto& region : imageRegions) {
		SVE::destroyImage(region.image.image);
		SVE::destroyImageView(region.image.view);
	}
	for (size_t i = 0; i < allocatedRegions.size(); ++i) {
		SVE::freeMemory(allocatedRegions[i]);
	}
}

