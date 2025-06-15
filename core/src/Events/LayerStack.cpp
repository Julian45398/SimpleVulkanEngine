#include "Layers/LayerStack.hpp"


namespace SGF {
    LayerStack LayerStack::s_MainStack;

    void LayerStack::push(Layer* pLayer) {
        assert(pLayer != nullptr);
        auto& layer = *pLayer;
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
        delete layers[layerCount];
        if (layerCount != layers.size()) {
            // has overlay
            layers.erase(layers.begin() + layerCount);
        } else {
            layers.pop_back();
        }
    }
    void LayerStack::pushOverlay(Layer* pLayer) {
        assert(pLayer != nullptr);
        auto& layer = *pLayer;
        assert(layer.layerIndex == SIZE_MAX);
        layers.push_back(pLayer);
        layer.layerIndex = layers.size();
        layer.onAttach();
    }
    void LayerStack::popOverlay() {
        layers.back()->onDetach();
        auto pLayer = layers.back();
        layers.back()->layerIndex = SIZE_MAX;
        delete pLayer;
        layers.pop_back();
    }
    void LayerStack::insert(Layer* pLayer, size_t index) {
        assert(pLayer != nullptr);
        auto& layer = *pLayer;
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
    void LayerStack::clear() {
        for (auto& layer : layers) {
            layer->onDetach();
            layer->layerIndex = SIZE_MAX;
            delete layer;
        }
        layers.clear();
        layers.shrink_to_fit();
    }

    void LayerStack::onEvent(RenderEvent& event) {
        for (auto& lay : layers) {
            lay->onRender(event);
        }
    }
    void LayerStack::onEvent(const UpdateEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            layers[i - 1]->onUpdate(event);
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