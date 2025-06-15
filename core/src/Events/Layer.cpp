#include "Layers/Layer.hpp"
#include "Layers/LayerStack.hpp"


namespace SGF {
    Layer::Layer(const Layer& other) {
        SGF::debug("Layer copy constructor called!");
        layerIndex = UINT64_MAX;
        debugName = other.debugName;
    }
    /*
    Layer::Layer(Layer&& other) noexcept {
        SGF::debug("Layer move constructor called!");
        layerIndex = other.layerIndex;
        debugName = other.debugName;
        if (other.layerIndex != UINT64_MAX) {
            auto it = std::find(LayerStack::layers.begin(), LayerStack::layers.end(), &other);
            if (it != LayerStack::layers.end()) {
                LayerStack::layers.at(std::distance(LayerStack::layers.begin(), it)) = this;
            } else {
                SGF::error("layer should be able to find itself in the layer stack!");
            }
        }
        other.layerIndex = UINT64_MAX;
    }
    */
    Layer::~Layer() {
        SGF::debug("Layer destructor called!");
        if (layerIndex != UINT64_MAX) {
            //auto it = std::find(LayerStack::layers.begin(), LayerStack::layers.end(), this);
            //if (it != LayerStack::layers.end()) {
                //onDetach();
                //LayerStack::layers.erase(it);
            //} else {
                //SGF::error("layer should be able to find itself in the layer stack!");
            //}
            layerIndex = UINT64_MAX;
        }
    }
}