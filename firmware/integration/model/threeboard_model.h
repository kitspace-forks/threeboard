#pragma once

#include "integration/model/layer_model.h"
#include "integration/model/observable_state.h"
#include "src/keypress.h"
#include "src/layers/layer_id.h"

namespace threeboard {
namespace integration {

class ThreeboardModel {
 public:
  ThreeboardModel()
      : dflt_layer_model_(&usb_buffer_),
        r_layer_model_(&usb_buffer_),
        g_layer_model_(&usb_buffer_),
        b_layer_model_(&usb_buffer_) {}
  void Apply(const Keypress& keypress);
  ObservableState GetStateSnapshot();

 private:
  LayerModel* CurrentLayerModel();

  std::string usb_buffer_;
  DefaultLayerModel dflt_layer_model_;
  DefaultLayerModel r_layer_model_;
  DefaultLayerModel g_layer_model_;
  DefaultLayerModel b_layer_model_;
  LayerId current_layer_;
};
}  // namespace integration
}  // namespace threeboard