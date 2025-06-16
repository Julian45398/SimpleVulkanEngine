#include "SGF.hpp"

void SGF::PreInit() {
	SGF::Device::RequireFeatures(DEVICE_FEATURE_GEOMETRY_SHADER | DEVICE_FEATURE_TESSELLATION_SHADER);
	SGF::Device::RequireGraphicsQueues(1);
	SGF::Device::RequireTransferQueues(1);
	SGF::Window::SetCreateFlags(WINDOW_FLAG_RESIZABLE | WINDOW_FLAG_NO_COLOR_CLEAR);
	SGF::Window::SetTitle("How are you?");
	SGF::Window::SetSize(1280, 800);
	//SGF::Window::SetMultisample();
	//SGF::Window::AddDepthImage(VK_FORMAT_D16_UNORM, 1.f, 0);
	//SGF::Window::SetSampleCount(VK_SAMPLE_COUNT_1_BIT);
	//SGF::Window::EnableClear(0.f, 0.f, 0.f, 0.f);
	//SGF::Window::DisableClear(0.f, 0.f, 0.f, 0.f);
}

void SGF::Setup() {
	LayerStack::PushOverlay(new ImGuiLayer(VK_SAMPLE_COUNT_1_BIT));
	LayerStack::Push(new ViewportLayer(VK_FORMAT_R8G8B8A8_SRGB));
}
void SGF::Cleanup() {

}
