#include "SGF.hpp"

void SGF::PreInit() {
	//SGF::SetWindowTitle("Hello World from Preinit");
	//SGF::SetWindowFlags(SGF::WINDOW_FLAG_RESIZABLE);
	//SGF::SetDeviceGraphicsQueueCount(1);
	//SGF::SetDeviceTransferQueueCount(1);
		//SGF::SetDeviceRequiredFeatures(requiredFeatures, ARRAY_SIZE(requiredFeatures));
	SGF::Device::RequireFeatures(DEVICE_FEATURE_GEOMETRY_SHADER | DEVICE_FEATURE_TESSELLATION_SHADER);
	SGF::Device::RequireGraphicsQueues(1);
	SGF::Device::RequireTransferQueues(1);
}

void SGF::Setup() {

}
void SGF::Cleanup() {

}
