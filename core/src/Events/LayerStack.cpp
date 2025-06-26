#include "Layers/LayerStack.hpp"


namespace SGF {
    LayerStack LayerStack::s_MainStack;

    void LayerStack::Push(Layer* pLayer) {
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
        layer.OnAttach();
    }
    void LayerStack::Pop() {
        assert(layerCount != 0);
        layerCount--;
        assert(layers[layerCount]->layerIndex != SIZE_MAX);
        layers[layerCount]->OnDetach();
        layers[layerCount]->layerIndex = SIZE_MAX; 
        delete layers[layerCount];
        if (layerCount != layers.size()) {
            // has overlay
            layers.erase(layers.begin() + layerCount);
        } else {
            layers.pop_back();
        }
    }
    void LayerStack::PushOverlay(Layer* pLayer) {
        assert(pLayer != nullptr);
        auto& layer = *pLayer;
        assert(layer.layerIndex == SIZE_MAX);
        layers.push_back(pLayer);
        layer.layerIndex = layers.size();
        layer.OnAttach();
    }
    void LayerStack::PopOverlay() {
        layers.back()->OnDetach();
        auto pLayer = layers.back();
        layers.back()->layerIndex = SIZE_MAX;
        delete pLayer;
        layers.pop_back();
    }
    void LayerStack::Insert(Layer* pLayer, size_t index) {
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
        layer.OnAttach();
    }
    void LayerStack::Clear() {
        for (auto& layer : layers) {
            layer->OnDetach();
            layer->layerIndex = SIZE_MAX;
            delete layer;
        }
        layers.clear();
        layers.shrink_to_fit();
    }

    void LayerStack::OnEvent(RenderEvent& event) {
        for (auto& lay : layers) {
            lay->OnEvent(event);
        }
    }
    void LayerStack::OnEvent(const UpdateEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            layers[i - 1]->OnEvent(event);
        }
    }
    void LayerStack::OnEvent(const KeyPressedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
    void LayerStack::OnEvent(const KeyReleasedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
    void LayerStack::OnEvent(const KeyRepeatEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
    void LayerStack::OnEvent(const KeyTypedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
    void LayerStack::OnEvent(const MousePressedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
    void LayerStack::OnEvent(const MouseReleasedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
    void LayerStack::OnEvent(const MouseMovedEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
    void LayerStack::OnEvent(const MouseScrollEvent& event) {
        for (size_t i = layers.size(); i != 0; --i) {
            if (layers[i-1]->OnEvent(event)) {
                break;
            }
        }
    }
}