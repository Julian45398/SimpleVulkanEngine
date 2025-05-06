#include "SVE_Backend.h"

#include "SVE_Entrypoint.hpp"

#include "SGF.hpp"

namespace SGF {
	template<typename, typename = std::void_t<>>
	struct has_keyPressed : std::false_type {};
	template<typename T>
	struct has_keyPressed<T, std::void_t<decltype(static_cast<void(*)(KeyPressedEvent&, T*)>(&T::onKeyPressed))>> : std::true_type {};

	class hasKeyPressed {
	public:
		static void onKeyPressed(KeyPressedEvent& event, hasKeyPressed* user) {
			info("Hello world!");
		}
	};


	class hasKeyRepeat {
	public:
		static void onKeyPressed(KeyRepeatEvent& event, hasKeyPressed* user) {
			info("hello world!");
		}
	};
}


int main() {

	SGF::TestLayer layer(66);
	//SGF::STACK.pushLayer(layer);
	SGF::TestLayer* objPtr = &layer;
	using TestFunc = void (SGF::TestLayer::*)();
	using BaseFunc = void (SGF::Layer::*)();
	SGF::LayerStack::push(layer);
	//static_assert(SGF::has_keyPressed<SGF::hasKeyPressed>::value, "A has correct static foo(int)");
	//static_assert(!SGF::has_keyPressed<SGF::hasKeyRepeat>::value, "B has foo but wrong signature");
	if constexpr (SGF::has_keyPressed<SGF::hasKeyPressed>::value) {
		SGF::info("has key pressed!");
	} else {
		SGF::error("should have key pressed!");
	}

	if constexpr (SGF::has_keyPressed<SGF::hasKeyRepeat>::value) {
		SGF::error("should not happen!");
	} else {
		SGF::info("is missing key repeat!");
	}

	if (false) {
		SGF::init();
		{
			SGF::Window window("Hello world!", 600, 400, SGF::WINDOW_FLAG_RESIZABLE);
			SGF::Device device = SGF::Device::Builder().bindWindow(&window).graphicQueues(1).computeQueues(0).transferQueues(1)
				.requireFeature(SGF::DEVICE_FEATURE_GEOMETRY_SHADER).build();
			
			//SGF::WindowEvents.subscribe(onEvent, &window);
			while (!window.shouldClose()) {
				window.onUpdate();
			}
		}
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