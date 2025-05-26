
#include "SGF.hpp"
//#include "SVE_Backend.h"
//#include "SVE_Entrypoint.hpp"

void onWindowClose(const SGF::WindowCloseEvent& event, char* listener) {
	SGF::info("Hello on window close called!", listener);
}

int main() {
	if (true) {
		SGF::init();
		{
			SGF::Window window("Hello world!", 600, 400, SGF::WINDOW_FLAG_RESIZABLE);
			SGF::pickDevice().forWindow(window).graphicQueues(1).transferQueues(1)
				.requireFeature(SGF::DEVICE_FEATURE_GEOMETRY_SHADER).build();
			const SGF::Device& device = SGF::getDevice();
			VkSampleCountFlagBits multisampleCount = VK_SAMPLE_COUNT_1_BIT;//device.getMaxSupportedSampleCount();
			const std::vector<VkAttachmentDescription> attachments = {
				SGF::Swapchain::createAttachment(device, window, VK_ATTACHMENT_LOAD_OP_CLEAR),
				SGF::createDepthAttachment(VK_FORMAT_D16_UNORM, multisampleCount)
				//SGF::createAttachmentDescription(device.pickSurfaceFormat(window, SGF::Swapchain::DEFAULT_SURFACE_FORMAT).format, multisampleCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR)
			};
			VkAttachmentReference swapchainRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkAttachmentReference depthRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			//VkAttachmentReference colorRef = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			const std::vector<VkSubpassDescription> subpasses = {
				SGF::createSubpassDescription(&swapchainRef, 1, nullptr, &depthRef)
			};
			const std::vector<VkSubpassDependency> dependencies = {
				{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT }
			};
			window.setRenderPass(attachments, subpasses, dependencies);

			VkPipelineLayout layout = device.pipelineLayout(0, nullptr, 0, nullptr);
			VkPipeline pipeline = device.graphicsPipeline(layout, window.getRenderPass(), 0).dynamicState(VK_DYNAMIC_STATE_SCISSOR).dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
				.vertexShader("shaders/test_triangle.vert").fragmentShader("shaders/test_triangle.frag").sampleCount(multisampleCount).build();

			VkFence renderFence = device.fenceSignaled();
			VkSemaphore imageAvailable = device.semaphore();
			VkSemaphore renderFinished = device.semaphore();
			SGF::CommandList commands(device, SGF::QUEUE_TYPE_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
			const std::vector<VkClearValue> clearValues = {
				SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
				SGF::createDepthClearValue(1.0f, 0)
				//SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
			};
			SGF::Timer timer;

			SGF::ImGuiLayer imGuiLayer(window, multisampleCount);
			SGF::LayerStack::pushOverlay(imGuiLayer);
			double deltaTime = 0.0;
			while (!window.shouldClose()) {
				window.onUpdate();
				device.waitFence(renderFence);
				device.reset(renderFence);
				window.nextFrame(imageAvailable, VK_NULL_HANDLE);
				//SGF::LayerStack::onEvent()
				commands.reset();
				commands.begin();
				SGF::RenderEvent renderEvent(deltaTime, commands);
				SGF::UpdateEvent updateEvent(deltaTime);
				SGF::LayerStack::onEvent(updateEvent);
				commands.beginRenderPass(window, clearValues);
				commands.bindGraphicsPipeline(pipeline);
				commands.setRenderArea(window);
				commands.draw(3);
				SGF::LayerStack::onEvent(renderEvent);
				commands.endRenderPass();
				commands.end();
				commands.submit(imageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, renderFinished, renderFence);
				window.presentFrame(renderFinished);
			}
			device.waitIdle();
			device.destroy(renderFence, imageAvailable, renderFinished, layout, pipeline);
		}
		SGF::shutdownDevice();
		SGF::terminate();
	}
	/*
	if (false) {
		shl::logDebug("Version: ", PROJECT_VERSION);
		SVE::init(1280, 720);
		SVE::run();
		SVE::terminate();
	}
	*/

	return 0;
}