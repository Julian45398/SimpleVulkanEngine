#include "Render/ImageMemoryAllocator.hpp"
#include "Render/Device.hpp"

namespace SGF {
	const TextureImage ImageMemoryAllocator::CreateImage(uint32_t width, uint32_t height) {
		auto& device = Device::Get();
		TextureImage texture;
		MemRegion textureRegion;
		texture.image = device.CreateImage2D(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		auto memreq = device.GetMemoryRequirements(texture.image);
		if (memreq.size > REGION_SIZE) {
			fatal("image memory requirement exceeds max byts size of: ", REGION_SIZE);
		}
		bool region_found = false;
		for (size_t i = 0; i < freeRegions.size(); ++i) {
			if (memreq.size <= freeRegions[i].size) {
				auto region = freeRegions[i];
				uint32_t alignmentOffset = region.offset % (uint32_t)memreq.alignment;
				if (alignmentOffset != 0) {
					uint32_t correction = (uint32_t)memreq.alignment - alignmentOffset;
					if ((region.size - correction) < memreq.size) continue;
					MemRegion inbetweenRegion;
					inbetweenRegion.offset = region.offset;
					inbetweenRegion.regionIndex = region.regionIndex;
					inbetweenRegion.size = correction;
					region.offset += correction;
					freeRegions.push_back(inbetweenRegion);
				}
				region.size = memreq.size;
				region_found = true;
				device.BindMemory(allocatedRegions[region.regionIndex], texture.image, region.offset);
				region.size = (uint32_t)memreq.size;
				textureRegion = region;
				if ((memreq.size + alignmentOffset) == freeRegions[i].size) {
					freeRegions.erase(freeRegions.begin() + i);
				} else {
					freeRegions[i].offset += (alignmentOffset + (uint32_t)memreq.size);
					freeRegions[i].size -= ((uint32_t)memreq.size + alignmentOffset);
				}
				break;
			}
		}
		if (!region_found) {
			uint32_t size = (uint32_t)memreq.size;
			memreq.size = REGION_SIZE;
			VkDeviceMemory memory = device.AllocateMemory(memreq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			allocatedRegions.push_back(memory);
			MemRegion region = { size, 0, (uint32_t)allocatedRegions.size() - 1 };
			device.BindMemory(memory, texture.image, region.offset);
			textureRegion = region;
			region.offset = size;
			region.size = (uint32_t)REGION_SIZE - size;
			freeRegions.push_back(region);
		}
		texture.view = device.CreateImageView2D(texture.image, VK_FORMAT_R8G8B8A8_SRGB);
		imageRegions.push_back({texture, textureRegion});
		return imageRegions.back().image;
	}
	const TextureImage ImageMemoryAllocator::CreateDummyImage(uint32_t width, uint32_t height) {
		auto& device = Device::Get();
		TextureImage texture;
		MemRegion textureRegion;
		texture.image = device.CreateImage2D(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		auto memreq = device.GetMemoryRequirements(texture.image);
		if (memreq.size > REGION_SIZE) {
			fatal("image memory requirement exceeds max byts size of: ", REGION_SIZE);
		}
		bool region_found = false;
		for (size_t i = 0; i < freeRegions.size(); ++i) {
			if (memreq.size <= freeRegions[i].size) {
				auto region = freeRegions[i];
				region_found = true;
				device.BindMemory(allocatedRegions[region.regionIndex], texture.image, region.offset);
				break;
			}
		}
		if (!region_found) {
			uint32_t size = (uint32_t)memreq.size;
			memreq.size = REGION_SIZE;
			VkDeviceMemory memory = device.AllocateMemory(memreq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			allocatedRegions.push_back(memory);
			MemRegion region = { REGION_SIZE, 0, (uint32_t)allocatedRegions.size() - 1 };
			device.BindMemory(memory, texture.image, region.offset);
			freeRegions.push_back(region);
		}
		texture.view = device.CreateImageView2D(texture.image, VK_FORMAT_R8G8B8A8_SRGB);
		return texture;
	}


	void ImageMemoryAllocator::DestroyImage(const TextureImage& texture) {
		auto& device = Device::Get();
		// If the texture is a dummy texture no region was bound to it so 
		MemRegion region;
		for (uint32_t i = 0; i < imageRegions.size(); ++i) {
			if (imageRegions[i].image.image ==  texture.image) {
				region = imageRegions[i].region;
				imageRegions.erase(imageRegions.begin() + i);
				freeRegions.push_back(region);
				break;
			}
		}
		device.Destroy(texture.image, texture.view);
	}

	void ImageMemoryAllocator::DefragmentMemory() {
		warn("Defragmentation of image allocator memory not implemented yet!");
	}

	ImageMemoryAllocator::~ImageMemoryAllocator() {
		auto& device = Device::Get();
		for (const auto& region : imageRegions) {
			device.Destroy(region.image.image, region.image.view);
		}
		for (size_t i = 0; i < allocatedRegions.size(); ++i) {
			device.Destroy(allocatedRegions[i]);
		}
	}
}