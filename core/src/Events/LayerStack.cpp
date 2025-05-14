#include "Layers/LayerStack.hpp"


namespace SGF {
    size_t LayerStack::layerCount = 0;

    void LayerStack::push(Layer& layer) {
        assert(layer.layerIndex == SIZE_MAX);
        if (layerCount != layers.size()) {
            // has overlay
            layers.insert(layers.begin() + layerCount, &layer);
        } else {
            layers.push_back(&layer);
        }
        layer.layerIndex = layerCount;
        layerCount++;
        layer.onAttach();
    }
    void LayerStack::pop() {
        assert(layerCount != 0);
        layerCount--;
        assert(layers[layerCount]->layerIndex != SIZE_MAX);
        layers[layerCount]->onDetach();
        layers[layerCount]->layerIndex = SIZE_MAX; 
        if (layerCount != layers.size()) {
            // has overlay
            layers.erase(layers.begin() + layerCount);
        } else {
            layers.pop_back();
        }
    }
    void LayerStack::pushOverlay(Layer& layer) {
        assert(layer.layerIndex == SIZE_MAX);
        layers.push_back(&layer);
        layer.layerIndex = layers.size();
        layer.onAttach();
    }
    void LayerStack::popOverlay() {
        layers.back()->onDetach();
        layers.back()->layerIndex = SIZE_MAX;
        layers.pop_back();
    }
    void LayerStack::insert(Layer& layer, size_t index) {
        assert(layer.layerIndex == SIZE_MAX);
        assert(index < layerCount);
        layers.insert(layers.begin() + index, &layer);
        for (size_t i = index; i < layers.size(); ++i) {
            assert(i != layers[i]->layerIndex);
            layers[i]->layerIndex = i;
        }
        layerCount++;
        layer.onAttach();
    }
    void LayerStack::erase(Layer& layer) {
        assert(layer.layerIndex != SIZE_MAX);
        auto it = std::find(layers.begin(), layers.begin() + layerCount, &layer);
		if (it != layers.begin() + layerCount) {
			layer.onDetach();
            layer.layerIndex = SIZE_MAX;
			layers.erase(it);
			layerCount--;
            for (size_t i = std::distance(layers.begin(), it); i < layers.size(); ++i) {
                assert(layers[i]->layerIndex != i);
                layers[i]->layerIndex = i;
            }
		}
    }
    void LayerStack::clear() {
        for (auto& layer : layers) {
            layer->onDetach();
            layer->layerIndex = SIZE_MAX;
        }
        layers.clear();
        layers.shrink_to_fit();
    }

    void LayerStack::onEvent(const RenderEvent& event) {
        for (auto& lay : layers) {
            lay->onRender(event);
        }
    }
    void LayerStack::onEvent(const UpdateEvent& event) {
        for (auto& lay : layers) {
            lay->onUpdate(event);
        }
    }
    void LayerStack::onEvent(const KeyPressedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onKeyPress(event)) {
                break;
            }
        }
    }
    void LayerStack::onEvent(const KeyReleasedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onKeyRelease(event)) {
                break;
            }
        }
    }
    void LayerStack::onEvent(const KeyRepeatEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onKeyRepeat(event)) {
                break;
            }
        }
    }
    void LayerStack::onEvent(const KeyTypedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onKeyTyped(event)) {
                break;
            }
        }
    }
    void LayerStack::onEvent(const MousePressedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onMousePress(event)) {
                break;
            }
        }
    }
    void LayerStack::onEvent(const MouseReleasedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onMouseRelease(event)) {
                break;
            }
        }
    }
    void LayerStack::onEvent(const MouseMovedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onMouseMove(event)) {
                break;
            }
        }
    }
    void LayerStack::onEvent(const MouseScrollEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->onMouseScroll(event)) {
                break;
            }
        }
    }
}