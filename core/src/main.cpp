#include "SVE_Backend.h"

#include "SVE_Entrypoint.hpp"

#include "SGF.hpp"

void onWindowClose(const SGF::WindowCloseEvent& event, char* listener) {
	SGF::info("Hello on window close called!", listener);
}

int main() {
	SGF::TestLayer layer(66);
	SGF::LayerStack::push(layer);
	char* helloWorld = "how are you today?";
	SGF::EventManager::addListener(onWindowClose, helloWorld);

	if (true) {
		SGF::init();
		{
			SGF::Window window("Hello world!", 600, 400, SGF::WINDOW_FLAG_RESIZABLE);
			SGF::pickDevice().forWindow(window).graphicQueues(1).computeQueues(0).transferQueues(1)
				.requireFeature(SGF::DEVICE_FEATURE_GEOMETRY_SHADER).build();
			const SGF::Device& device = SGF::getDevice();
			VkAttachmentDescription attachments[] = {
				{0, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR}
			};
			VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkSubpassDescription subpasses[] = {
				SGF::createSubpassDescription(VK_SAMPLE_COUNT_1_BIT, &colorRef, 1)
			};
			SGF::Swapchain swapchain(device, window, VK_PRESENT_MODE_MAILBOX_KHR, attachments, ARRAY_SIZE(attachments), subpasses, ARRAY_SIZE(subpasses));

			VkPipelineLayout layout = device.pipelineLayout(0, nullptr, 0, nullptr);
			VkPipeline pipeline = device.graphicsPipeline(layout, swapchain.getRenderPass(), 0).dynamicState(VK_DYNAMIC_STATE_SCISSOR).dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
				.vertexShader("shaders/test_triangle.vert").fragmentShader("shaders/test_triangle.frag").build();

			VkFence windowFence = device.fence();
			VkSemaphore imageAvailable = device.semaphore();
			VkCommandPool renderpool = device.commandPool(device.graphicsFamily(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
			VkCommandBuffer rendercommands = device.commandBuffer(renderpool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			while (!window.shouldClose()) {
				window.onUpdate();
				swapchain.nextImage(imageAvailable, windowFence);
				swapchain.presentImage(imageAvailable);
				break;
			}
			swapchain.destroy();
			device.waitIdle();
			device.destroy(windowFence, imageAvailable, layout, pipeline);
			window.close();
		}
		SGF::shutdownDevice();
		SGF::terminate();
	}
	if (false) {
		shl::logDebug("Version: ", PROJECT_VERSION);
		SVE::init(1280, 720);
		SVE::run();
		SVE::terminate();
	}

	return 0;
}