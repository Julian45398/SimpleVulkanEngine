#pragma once

#include "SGF_Core.hpp"
#include "Device.hpp"

namespace SGF {
	template <typename T>
	class UniformArray {
	private:
		VkDeviceMemory deviceMemory;
		std::vector<VkBuffer> buffers;
		uint8_t* mappedMemory;
		VkDeviceSize uniformSize;
	public:
		inline UniformArray(uint32_t count) {
			buffers.resize(count);
			auto& device = Device::Get();
			for (size_t i = 0; i < buffers.size(); ++i) {
				buffers[i] = device.CreateBuffer(sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			}
			VkMemoryRequirements req = device.GetMemoryRequirements(buffers[0]);
			uniformSize = req.size;
			req.size = count * req.size;
			deviceMemory = device.AllocateMemory(req, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			for (size_t i = 0; i < buffers.size(); ++i) {
				device.BindMemory(deviceMemory, buffers[i], i * uniformSize);
			}
			mappedMemory = (uint8_t*)device.MapMemory(deviceMemory);
		}
		inline ~UniformArray() {
			auto& device = Device::Get();
			for (size_t i = 0; i < buffers.size(); ++i) {
				device.Destroy(buffers[i]);
			}
			device.Destroy(deviceMemory);
		}
		inline VkDescriptorSetLayoutBinding GetDescriptorLayoutBinding(uint32_t binding, VkShaderStageFlags shaderStage) {
			return { binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, shaderStage, nullptr };
		}
		inline void SetValue(const T& newValue, uint32_t index) {
			size_t offset = index * uniformSize;
			memcpy(mappedMemory + offset, &newValue, sizeof(T));
		}
		inline VkBuffer GetBuffer(uint32_t index) { return buffers[index]; }
		inline operator const VkBuffer*() { return buffers.data(); }
		inline VkDeviceSize Size() { return uniformSize; }
		inline const VkBuffer* GetBuffers() { return buffers.data(); }
	};
}