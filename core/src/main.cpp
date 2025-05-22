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
			const std::vector<VkAttachmentDescription> attachments[] = {
				SGF::createAttachmentDescription(device.getSwapchainFormat(window), VK_SAMPLE_COUNT_1_BIT, 
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			};
			VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			const std::vector<VkSubpassDescription> subpasses = {
				SGF::createSubpassDescription(VK_SAMPLE_COUNT_1_BIT, &colorRef, 1)
			};
			SGF::Swapchain swapchain(device, window, VK_PRESENT_MODE_FIFO_KHR, attachments, ARRAY_SIZE(attachments), subpasses, ARRAY_SIZE(subpasses));

			VkPipelineLayout layout = device.pipelineLayout(0, nullptr, 0, nullptr);
			VkPipeline pipeline = device.graphicsPipeline(layout, swapchain.getRenderPass(), 0).dynamicState(VK_DYNAMIC_STATE_SCISSOR).dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
				.vertexShader("shaders/test_triangle.vert").fragmentShader("shaders/test_triangle.frag").build();

			VkFence renderFence = device.fenceSignaled();
			VkSemaphore imageAvailable = device.semaphore();
			VkSemaphore renderFinished = device.semaphore();
			SGF::CommandList commands(device, SGF::QUEUE_TYPE_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
			auto clearValue = SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f);
			while (!window.shouldClose()) {
				window.onUpdate();
				device.waitFence(renderFence);
				device.reset(renderFence);
				swapchain.nextImage(imageAvailable, VK_NULL_HANDLE);
				commands.reset();
				commands.begin();
				commands.beginRenderPass(swapchain, &clearValue, 1);
				commands.bindGraphicsPipeline(pipeline);
				commands.setRenderArea(swapchain);
				commands.draw(3);
				commands.endRenderPass();
				commands.end();
				commands.submit(imageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, renderFinished, renderFence);
				swapchain.presentImage(renderFinished);
				device.waitIdle();
				break;
			}
			swapchain.destroy();
			device.waitIdle();
			device.destroy(renderFence, imageAvailable, renderFinished, layout, pipeline, renderpool);
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