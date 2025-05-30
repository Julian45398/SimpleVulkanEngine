
#include "SGF.hpp"
//#include "SVE_Backend.h"
//#include "SVE_Entrypoint.hpp"

void onWindowClose(const SGF::WindowCloseEvent& event, char* listener) {
	SGF::info("Hello on window close called!", listener);
}

void SGF::Run() {
	SGF::Window window("Hello world!", 600, 400, SGF::WINDOW_FLAG_RESIZABLE);
	SGF::Device::PickNew().forWindow(window).graphicQueues(1).transferQueues(1)
		.requireFeature(SGF::DEVICE_FEATURE_GEOMETRY_SHADER).build();
	const SGF::Device& device = SGF::Device::Get();
	VkSampleCountFlagBits multisampleCount = VK_SAMPLE_COUNT_1_BIT;//device.getMaxSupportedSampleCount();
	const std::vector<VkAttachmentDescription> attachments = {
		SGF::Swapchain::createAttachment(device, window, VK_ATTACHMENT_LOAD_OP_CLEAR),
		//SGF::createDepthAttachment(VK_FORMAT_D16_UNORM, multisampleCount)
		//SGF::createAttachmentDescription(device.pickSurfaceFormat(window, SGF::Swapchain::DEFAULT_SURFACE_FORMAT).format, multisampleCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR)
	};
	VkAttachmentReference swapchainRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	//VkAttachmentReference depthRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	//VkAttachmentReference colorRef = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	const std::vector<VkSubpassDescription> subpasses = {
		SGF::createSubpassDescription(&swapchainRef, 1, nullptr, nullptr)
	};
	const std::vector<VkSubpassDependency> dependencies = {
		{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT }
	};
	window.setRenderPass(attachments, subpasses, dependencies);

	VkSemaphore imageAvailable = device.semaphore();
	VkSemaphore renderFinished = device.semaphore();
	SGF::CommandList commands(device, SGF::QUEUE_TYPE_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	const std::vector<VkClearValue> clearValues = {
		SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
		//SGF::createDepthClearValue(1.0f, 0)
		//SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
	};
	SGF::Timer timer;

	SGF::ImGuiLayer imGuiLayer(window, multisampleCount);
	SGF::ViewportLayer viewportLayer(VK_FORMAT_R8G8B8A8_SRGB);
	SGF::LayerStack::pushOverlay(imGuiLayer);
	SGF::LayerStack::push(viewportLayer);
	double deltaTime = 0.0;
	while (!window.shouldClose()) {
		window.onUpdate();
		SGF::UpdateEvent updateEvent(deltaTime);
		SGF::LayerStack::onEvent(updateEvent);
		commands.begin();
		window.nextFrame(imageAvailable, VK_NULL_HANDLE);
		//SGF::LayerStack::onEvent()
		//commands.beginRenderPass(window, clearValues);
		//commands.bindGraphicsPipeline(pipeline);
		//commands.setRenderArea(window);
		SGF::RenderEvent renderEvent(window, deltaTime, commands, window.getFramebufferSize());
		renderEvent.addWait(imageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		renderEvent.addSignal(renderFinished);
		SGF::LayerStack::onEvent(renderEvent);
		//commands.endRenderPass();
		commands.end();
		commands.submit(renderEvent.getWait().data(), renderEvent.getWaitStages().data(), renderEvent.getWait().size(), renderEvent.getSignal().data(), renderEvent.getSignal().size());
		window.presentFrame(renderFinished);
		deltaTime = timer.ellapsedMillis();
	}
	device.waitIdle();
	device.destroy(imageAvailable, renderFinished);
}

int main() {
	if (true) {
		SGF::Init();
		SGF::Run();
		SGF::Terminate();
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